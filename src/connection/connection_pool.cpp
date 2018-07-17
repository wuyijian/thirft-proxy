#include "connection_pool.h"
#include "log.h"

ConnectionPool::ConnectionPoolMap ConnectionPool::conn_pool_map_;


ConnectionPool::ConnectionPool(std::string server_name, std::vector<std::pair<std::string, int> > ip_port_pair)  
    : pool_size_(20),
      min_size_(18),
      max_idle_sec_(10),
      check_interval_sec_(5),
      count_(0),
      server_name_(server_name),
      ip_port_pair_(ip_port_pair)
{
}

ConnectionPool::~ConnectionPool()
{
    destroy();
}


/// 多个ip 和 port 是均匀分布的，可以起到负载均衡以及容灾的功能
void ConnectionPool::init()
{
    for (unsigned int i = 0; i < pool_size_ / ip_port_pair_.size(); ++i) 
    {
        for (unsigned int j = 0; j < ip_port_pair_.size(); ++j)
        {
            boost::shared_ptr<Connection> conn = boost::make_shared<Connection>(ip_port_pair_[j].first, ip_port_pair_[j].second);
            conn->init();
            connection_pool_.push_back(conn);
        }
    }

    /// 启动一个线程用来管理各个连接，5秒执行一次检查
   
}



boost::shared_ptr<Connection> ConnectionPool::getConnectionBySer(std::string server_name)
{
    do 
    {
        boost::shared_ptr<ConnectionPool> conn_pool = ConnectionPool::selectConnPool(server_name);
        if (conn_pool.get() == NULL)
        {
            ERROR_LOG("get conn_pool failed, server_name is %s", server_name.c_str());
            break;
        }
        boost::shared_ptr<Connection> conn = conn_pool->getConnection();
        if (conn.get() == NULL)
        {
            ERROR_LOG("get conn failed, server_name is %s", server_name.c_str());
            break;
        }
        return conn;
    }while(0);

    return boost::shared_ptr<Connection>();
}


void ConnectionPool::releaseConnectionBySer(std::string server_name, boost::shared_ptr<Connection> conn, bool state)
{
    boost::shared_ptr<ConnectionPool> conn_pool = ConnectionPool::selectConnPool(server_name);
    if (conn_pool.get() == NULL)
    {
        ERROR_LOG("get conn_pool failed, server_name is %s", server_name.c_str());
    }
    conn_pool->release(conn, state);
}

/// 获取连接需要互斥访问
boost::shared_ptr<Connection> ConnectionPool::getConnection()
{
    {
        boost::mutex::scoped_lock lock(mu_);
        int index = count_ % pool_size_;
        for (int i = 0; i < pool_size_; ++i)
        {
            if (!connection_pool_[index]->isAvailable())
            {
                index = (index + 1) % pool_size_;
                continue;
            }
            count_ ++;
            return connection_pool_[index];
        }
        // 连接不够的情况
        WARN_LOG("connection is not enough, try active more");
        for (int i = 0; i < pool_size_; ++ i)
        {
            if (!connection_pool_[i]->isBusy())
            {
                try {
                    connection_pool_[i]->getTransport()->open();
                    connection_pool_[i]->setBusy(true);
                    return connection_pool_[i];
                } catch(TException& tx) {
                    connection_pool_[i]->setDead(true);
                    // return boost::shared_ptr<Connection>();
                }
            }
        }
        return boost::shared_ptr<Connection>();
    }
}

/// 释放连接需要互斥问
/// status 此conn的使用状态 成功 true,  失败 false
void ConnectionPool::release(boost::shared_ptr<Connection> conn, bool status)
{
    {
        boost::mutex::scoped_lock lock(mu_);
        conn->setBusy(false);
        
        if (status) {
            conn->setDead(false);
        }
        else {
            // conn->setDead(true);
            /// 是否需要把连接池的连接全部重置
            /// 因为连接池里连接其实是一个后端
            for (int i = 0; i < pool_size_; ++i)
            {
                connection_pool_[i]->getTransport()->close();
                connection_pool_[i]->setDead(true);
            }

        }
        /// 检查总连接，如果不够的话，就继续尝试打开连接
        /// 需要确认thrift的connect的耗时
        int now = time(NULL);
        // DEBUG_LOG("availableCount: %d, min_size: %d, ischecktime_reached: %d", availableCount(), min_size_, now - last_checked_time_ > check_interval_sec_);
        // if (availableCount() < min_size_ && now - last_checked_time_ > check_interval_sec_)
        /// 应对有协议失败的场景,或者是连接数过少的场景
        if (false == status || availableCount() < min_size_)
        // if (availableCount() < min_size_)
        {
            if (now - last_checked_time_ > check_interval_sec_)
            {
                DEBUG_LOG("try active more connection");
                for (int i = 0; i < pool_size_; ++i)
                {
                    if (!connection_pool_[i]->isBusy())
                    {
                        try {
                             connection_pool_[i]->getTransport()->open();
                             connection_pool_[i]->setDead(false);
                         } catch (TException &tx) {
                             connection_pool_[i]->setDead(true);
                         }
                    }
                }
                last_checked_time_ = now;
            }
        }
    }
}

/// get activeCount()
int ConnectionPool::availableCount() const
{   
    int available_count = 0;
    for (int i = 0; i < pool_size_; ++i)
    {
        if (connection_pool_[i]->isAvailable())
        {
            available_count ++;
        }
    }
    return available_count;
}

/// 选择对应服务的连接池
boost::shared_ptr<ConnectionPool> ConnectionPool::selectConnPool(std::string server_name)
{
    if (conn_pool_map_.count(server_name))
    {
        return conn_pool_map_[server_name];
    } 
    return boost::shared_ptr<ConnectionPool>();
}

/// 插入对应服务的连接池
void ConnectionPool::addConnPool(boost::shared_ptr<ConnectionPool> conn_pool)
{
    // conn_pool_map_[conn_pool->getServerName()] = conn_pool;
    conn_pool_map_.insert(std::make_pair(conn_pool->getServerName(), conn_pool));
}

void ConnectionPool::destroy()
{
    for (int i = 0; i < pool_size_; ++i)
    {
        connection_pool_[i]->destroy();
    }
}

/// 检查连接池内连接的状态
//void ConnectionPool::checkConnAliave()
//{
//    {
//        boost::mutex::scoped_lock lock(mu_);
//        for (int i = 0; i < pool_size_; ++ i)
//        {
//            if (!connection_pool_[i]->isBusy())
//            {
//                try {
//                    connection_pool_[i]->getTransport()->open();
//                    connection_pool_[i]->setBusy(true);
//                    return connection_pool_[i];
//                } catch(TException& tx) {
//                    connection_pool_[i]->setDead(true);
//                    // return boost::shared_ptr<Connection>();
//                }
//            }
//        }
//        return boost::shared_ptr<Connection>();
//    }
//}




