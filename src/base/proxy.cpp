#include "proxy.h"
#include <string.h> 
#include "async_server.h"
#include "log.h"
#include "proto/proto.h"
#include "singleton.h"
#include "client_info.h"
#include "mempool.h"
#include "list.h"
#include <sys/resource.h>
#include <sys/time.h>
#include "timer.h"

#include "connection_pool.h"
#include "thread_pool.h"

#include "tis_proto_handler.h"
#include "cmd_handler.h"
#include "utils/net_utils.h"

mempool_t* g_mempool = NULL;
list_head_t *g_client_fd_list = NULL;
int g_max_open_fd = 100000;
list_head_t g_net_timer;
int g_timeout_s = 2;


/// TODO 后续用单例模式替换全局变量
/// TODO 管理协议到方法映射的全局表
static boost::shared_ptr<ThreadPool<ClientInfo*> > g_thread_pool;
static boost::shared_ptr<CmdHandler> g_cmd_handler;

int Proxy::InitFdList()
{
    struct rlimit limit;
    int ret = 0;
    ret = getrlimit(RLIMIT_NOFILE, &limit);
    if (ret < 0 || limit.rlim_max == RLIM_INFINITY) {
        DEBUG_LOG("getrlimit failed");
        g_max_open_fd = config_get_intval("max_open_fd", 100000);
    } else {
        g_max_open_fd = limit.rlim_max;
    }

    DEBUG_LOG("set max_open_fd = %u", g_max_open_fd);

    g_client_fd_list = (list_head_t *)malloc(
            sizeof(list_head_t) * g_max_open_fd);
    if (g_client_fd_list == NULL) {
        ERROR_LOG("malloc fd list failed, error = %s",
                strerror(errno));
        return -1;
    }

    int i = 0;
    for (i = 0; i < g_max_open_fd; i++) {
        INIT_LIST_HEAD(&g_client_fd_list[i]);
    }

    return 0;
}

int Proxy::Init()
{	
    // 用户在客户端关闭时清理其请求
    if (InitFdList()) {
        ERROR_LOG("InitFdList failed");
        return -1;
    }

    // 初始化内存池，尽量支持10000级别的QPS
    g_mempool = mempool_create(sizeof(ClientInfo), getpagesize() * 4096);
    if(!g_mempool)
    {
        ERROR_LOG("create mempool failed");
        return -1;
    }
    
    // 初始化timeout列表
    g_timeout_s = config_get_intval("timeout_s", 2);
    if (timer_init(&g_net_timer)) {
        ERROR_LOG("timer init failed");
        return -1;
    }
    
    /// 配置缓存
    const char* tmpbuf = NULL;
    

    /// 初始化连接, tis服务,反垃圾系统
    {
        tmpbuf = config_get_strval("tis_server_addr", NULL);
        if (NULL == tmpbuf) {
            ERROR_LOG("config get server addrs failed");
            return -1;
        }
        std::vector<std::pair<std::string, int> > ip_port_pairs = Common::string_to_ip_port_pairs(std::string(tmpbuf));
        boost::shared_ptr<ConnectionPool> conn_pool(new ConnectionPool("tis", ip_port_pairs));
        conn_pool->setPoolSize(config_get_intval("pool_size", 20));
        conn_pool->setMinSize(config_get_intval("min_size", 18));
        conn_pool->setCheckIntervalSec(config_get_intval("check_interval_sec", 1));
        conn_pool->init();
        ConnectionPool::addConnPool(conn_pool);

        tmpbuf = NULL;
    }

    {
        tmpbuf = config_get_strval("dbser_server_addr", NULL);
        if (NULL == tmpbuf) {
            ERROR_LOG("config get server addrs failed");
            return -1;
        }
        std::vector<std::pair<std::string, int> > ip_port_pairs = Common::string_to_ip_port_pairs(std::string(tmpbuf));
        boost::shared_ptr<ConnectionPool> conn_pool(new ConnectionPool("dbser", ip_port_pairs));
        conn_pool->setPoolSize(config_get_intval("pool_size", 10));
        conn_pool->setMinSize(config_get_intval("min_size", 8));
        conn_pool->setCheckIntervalSec(config_get_intval("check_interval_sec", 1));
        conn_pool->init();
        ConnectionPool::addConnPool(conn_pool);

        tmpbuf = NULL;
    }


    /// 注册方法
    g_cmd_handler = boost::make_shared<CmdHandler>();
    
    /// TODO 用配置文件处理
    g_cmd_handler->registerCmd(0x4702, tisProtoHandler);

    /// 初始化线程池
    int thread_num = config_get_intval("thread_num", 5);
    g_thread_pool = boost::make_shared<ThreadPool<ClientInfo*> >();
    g_thread_pool->start(thread_num);

    return 0;
}

int Proxy::Uninit() 
{
    // client fd链表
    if (g_client_fd_list) {
        free(g_client_fd_list);
        g_client_fd_list = NULL;
    }

    // 清理内存池
    if (g_mempool) {
        mempool_destroy(g_mempool);
        g_mempool = NULL;
    }
    
    timer_uninit(&g_net_timer);
    g_thread_pool->stop();
    

    return 0;
}

int Proxy::GetPkgLenSer(int fd, const char *buf, uint32_t len) 
{
    return 0;
}

/// 需要处理global的定时
void Proxy:: TimeEvent() 
{
    timer_check_event(&g_net_timer);
}


/// 此方法涉及到多线程互斥访问，需要互斥
void Proxy::SendToClient(int fd, uint32_t seq_num, uint32_t cmd_id, 
                         uint32_t status_code, uint32_t user_id)
{
    char out_buf[8192];
    ProtoHeader *out_header = (ProtoHeader*)out_buf;

    out_header->pkg_len = sizeof(ProtoHeader);
    out_header->seq_num = seq_num;
    out_header->cmd_id = cmd_id;
    out_header->status_code = status_code;
    out_header->user_id = user_id;
    {
        net_send_cli(fd, out_buf, out_header->pkg_len);
    }
    return;
}


/// 此方法设计多线程互斥访问，需要互斥
void Proxy::SendToClient(ClientInfo *ci, const char *buf, 
                        int len, uint32_t status_code)
{
    char out_buf[8192];
    ProtoHeader *out_header = (ProtoHeader*)out_buf;

    out_header->pkg_len = sizeof(ProtoHeader) + len;
    out_header->seq_num = ci->cli_seq;
    out_header->cmd_id = ci->cli_cmd;
    out_header->status_code = status_code;
    out_header->user_id = ci->userid;
    
    // TODO 注意限制包长
    if (len > 0) {
        memcpy(out_buf + sizeof(ProtoHeader), buf, len);
    }

    net_send_cli(ci->cli_fd, out_buf, out_header->pkg_len);
    // DestroyClient(ci->ser_seq);
    return;
}

void Proxy::ProcPkgCli(int fd, const char *buf, uint32_t len) 
{
    /// 此时一定Proxy::Init()已经成功了
    // DEBUG_LOG("Proxy ProcPkgCli start");
    if (len < 0 || len > 8192) {
        ERROR_LOG("cli pakage len err, len %d", len);
        return;
    }
    ProtoHeader * in_header = (ProtoHeader*)buf;   
    
    ClientInfo *ci = (ClientInfo*)mempool_calloc(g_mempool);

    if (!ci) {
        ERROR_LOG("Proxy ProcPkgCli failed for get NULL from mempool");
        SendToClient(fd, in_header->seq_num, in_header->cmd_id, 
                1001, in_header->user_id);
        return;
    }

    event_init(&ci->event);

    
    gettimeofday(&ci->proc_time, NULL);

    ci->cli_seq = in_header->seq_num;
    ci->cli_fd = fd;
    ci->cli_cmd = in_header->cmd_id;
    ci->userid = in_header->user_id;
    ci->len = len - sizeof(ProtoHeader);   
    memset(ci->buf, 0, sizeof(ci->buf));
    
    /// 只是含有包体
    memcpy(ci->buf, buf + sizeof(ProtoHeader), len - sizeof(ProtoHeader));

    uint32_t backend_sn = ClientInfo::SetClientInfo(ci);

    list_add_tail(&ci->fd_list_node,  &g_client_fd_list[fd]);
    
    /// 加入定时器链表，开始计时，10s后删除
    timer_add_event(&g_net_timer,
            &ci->event,
            time(NULL) + g_timeout_s,
            Proxy::DoServerTimeout,
            ci,
            NULL);

    // DEBUG_LOG("Proxy ProcPkgCli get backend_sn %d", backend_sn);
    
    /// 注册回调函数
    /// TODO 从map中获取回调
    ci->accept(g_cmd_handler->getHandler(ci->cli_cmd));
    
    /// 交给线程池
    g_thread_pool->put(ci);

}

/// destroy clientinfo
void Proxy::DestroyClient(uint32_t sn)
{
    ClientInfo *ci = ClientInfo::GetClientInfo(sn);

    if (!ci) {
        ERROR_LOG("Proxy DestroyClient failed for get NULL ClientInfo by sn %u", sn);
        return;
    }

    // 从fd链表中删除
    list_del_init(&ci->fd_list_node);

    // 退回内存池
    mempool_free(g_mempool, ci);

    // 从unordered_map中删除
    ClientInfo::RemoveClientInfo(sn);
    
    // 从timeout中删除
    timer_del_event(&ci->event);
}

// 同步的service不会访问到这个函数，一定是异步的
void Proxy:: ProcPkgSer(int fd, const char *buf, uint32_t len) 
{
}

void Proxy:: LinkUpCli(int fd, uint32_t ip) 
{
    
}

void Proxy:: LinkUpSer(int fd, uint32_t ip, uint16_t port) 
{
    DEBUG_LOG("Proxy LinkUpSer fd is %d, ip is %u, port is %d", fd, ip, port);
}

void Proxy:: LinkDownCli(int fd) 
{
    if (fd < 0 || fd >= g_max_open_fd) {
        ERROR_LOG("Proxy LinkDownCli recv invalid fd %d", fd);
        return;
    }
    // list_head_t *pos, *next;
    // list_for_each_safe(pos, next, &g_client_fd_list[fd]) {
    //     client_list_node_t *node = (client_list_node_t *)pos;
    //     client_info_t *client = (client_info_t *)node->owner;

    //     
    //     
    //     DEBUG_LOG("client close fd = %d:0x%04X:%u",
    //               fd, client->cli_cmd, client->userid);
    //     node->owner = NULL;
    //     DestroyClient(client->cli_seq);
    // }

    ClientInfo *entry = NULL;
    ClientInfo *next = NULL;
    list_for_each_entry_safe(entry, next, &g_client_fd_list[fd], fd_list_node, ClientInfo) {
        DEBUG_LOG("destroy clientinfo %d, cmd 0x%04X, userid %u", 
                  fd, entry->cli_cmd, entry->userid);
        // DEBUG_LOG("start destroy");
        DestroyClient(entry->ser_seq);
    }
}

void Proxy:: LinkDownSer(int fd) 
{
    DEBUG_LOG("Proxy LinkDownSer fd is %d", fd);
}

/// 定期清理超时的clientinfo
int Proxy::DoServerTimeout(void *owner, void *arg)
{
    ///  这个超时处理函数需要保留
    ClientInfo *ci = (ClientInfo*)owner;
    // SendToClient(ci, NULL, 0, 2003);
    
    DestroyClient(ci->ser_seq);
    
    return 0;
}














