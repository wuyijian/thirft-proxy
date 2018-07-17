#include "tis_proto_handler.h"
#include "proxy.h"
#include "proto/proto.h"
#include "log.h"

#define  MAX_OUT_BUG_LEN 4096*2

void tisProtoHandler(ClientInfo *ci, boost::shared_ptr<ConnectionPool> conn_pool)
{

    DataBodyHuaXian * in_data = (DataBodyHuaXian *)(ci->buf);
    ContentBody * in_content = (ContentBody *)(ci->buf + sizeof(DataBodyHuaXian));
    std::string content = std::string(in_content->content);
    

    /// 获取 gameid 和 userid
    int gameid = in_data->game_id;
   
    uint8_t type = in_data->type;

    int userid = ci->userid;
    /// userid 转化为string, 兼容各种Id
    char buf[32] = {0};
    sprintf(buf, "%d", userid);
    std::string userid_string = std::string(buf);
    
    std::string tis_name = "tis";
    std::string dbser_name = "dbser";  

    int dirty_flag = 2; /// 未处理 疑似

    ReqCheckResult     dbser_req;
    RequestCheckStruct tis_req;
    CheckResultStruct  tis_res;

    /// 获取连接资源
    do {
        /// 小花仙的4类，不需要审，直接入库
        /// 后期估计也会要不需要审的
        if (type == 4 && gameid == 5)
        {
            Proxy::SendToClient(ci, NULL, 0, 1002);
            break;
        }
        boost::shared_ptr<Connection> tis_conn = ConnectionPool::getConnectionBySer(tis_name);
        if (tis_conn.get() == NULL)
        {
            ERROR_LOG("get conn failed, server_name: %s", tis_name.c_str());
            Proxy::SendToClient(ci, NULL, 0, 1001);
            break;
        }

        /// 与过滤引擎交互
        try {
            boost::shared_ptr<TProtocol> tis_protocol(new TBinaryProtocol(tis_conn->getTransport()));
            TisServiceClient tis_client(tis_protocol);
           
            tis_req.appid    =    1;
            tis_req.gameid   =    gameid;
            tis_req.dataid   =    "8888";
            tis_req.userid   =    userid_string;
            tis_req.content  =    content;
            tis_req.ip       =    "0.0.0.0";
            
            /// 测试压测
            tis_client.check(tis_res, tis_req);
            
            dirty_flag = tis_res.action;
            char out_buf[MAX_OUT_BUG_LEN] = {0};
            /// 成功后的release
            if (0 != tis_res.code)
            {
                /// 参数无意义，疑似
                /// 返回失败，作为疑似处理
                dirty_flag = 2;
            }

            memcpy(out_buf, &dirty_flag, sizeof(uint32_t));
            memcpy(out_buf + sizeof(uint32_t), ci->buf, ci->len);
            ConnectionPool::releaseConnectionBySer(tis_name, tis_conn, true);

            /// 加上包体,将游戏发送的包体信息再整合进去
            /// 只有game_id为5才做处理, 小花仙特殊需求需要加入消息内容，平台不建议这种接口
            if (5 == gameid) {
                Proxy::SendToClient(ci, (const char *)out_buf, ci->len + sizeof(uint32_t), 0);
            } else {
                Proxy::SendToClient(ci, (const char *)&dirty_flag, sizeof(uint32_t), 0);
            }
             
            /// 统计耗时，吞吐量测试
            struct timeval finish_time;
            struct timeval diff_time;

            gettimeofday(&finish_time, NULL);
            timersub(&finish_time, &ci->proc_time, &diff_time);

            DEBUG_LOG("sent to client len:%lu, message:%s, dirty_flag:%d, userid:%d, game_id:%d, type:%d, cost_time:%lu.%06lu sec",
                   ci->len + sizeof(uint32_t), content.c_str(), dirty_flag, userid, gameid, in_data->type, diff_time.tv_sec, diff_time.tv_usec);
            break;
            
        } catch (TException& tx) {
            ERROR_LOG("check error %s", tx.what());
            /// 失败后的release 
            /// TODO 释放连接
            ConnectionPool::releaseConnectionBySer(tis_name, tis_conn, false);
            /// TODO 错误码区分需要更加详细
            // Proxy::SendToClient(ci->cli_fd, ci->cli_seq, ci->cli_cmd, 1001, ci->userid);
            Proxy::SendToClient(ci, NULL, 0, 1001);
            break;
        }
    }while(0);
    
    /// 入库
    boost::shared_ptr<Connection> dbser_conn = ConnectionPool::getConnectionBySer(dbser_name);
    if (dbser_conn.get() == NULL)
    {
        ERROR_LOG("select conn failed, server_name: %s, dbser failed", dbser_name.c_str());
        return;
    }

    try {

        boost::shared_ptr<TProtocol> dbser_protocol(new TBinaryProtocol(dbser_conn->getTransport()));
        IdentityServClient dbser_client(dbser_protocol);
        
        dbser_req.message   =   content;
        dbser_req.flag      =   dirty_flag;
        dbser_req.appid     =   1;
        dbser_req.gameid    =   gameid;
        dbser_req.type      =   in_data->type;
        // dbser_req.itemid    =   in_data->item_id;
        dbser_req.serverid  =   in_data->home_id;
        dbser_req.userid    =   userid;
        dbser_req.rolegrade =   in_data->role_grade;
        dbser_req.vipgrade  =   in_data->vip_grade;
        dbser_req.nick      =   in_data->nick;

        dbser_client.StoreMessage(dbser_req);

        // DEBUG_LOG("finish store message message:%s, dirty_flag:%d, userid:%d, game_id:%d, type:%d",
        //                                    content.c_str(), dirty_flag, userid, gameid, in_data->type);

        ConnectionPool::releaseConnectionBySer(dbser_name, dbser_conn, true);
    } catch (TException& tx) {
        ERROR_LOG("store message error %s", tx.what());
        ConnectionPool::releaseConnectionBySer(dbser_name,dbser_conn, false);
        /// TODO 需要告警
    }   
}


