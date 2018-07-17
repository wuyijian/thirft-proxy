#ifndef __CONNECTION_POOL_H__ 
#define __CONNECTION_POOL_H__

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/move/move.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <assert.h>
#include <unordered_map>
#include <vector>

#include <cstddef>
#include "connection.h"


class ConnectionPool : boost::noncopyable
{
public: 
    /// 
    explicit ConnectionPool(std::string server_name, std::vector<std::pair<std::string, int> >);

    ~ConnectionPool();

    void init();

    void destroy();

    static boost::shared_ptr<Connection> getConnectionBySer(std::string);
    static void releaseConnectionBySer(std::string, boost::shared_ptr<Connection> conn, bool state);

    boost::shared_ptr<Connection> getConnection();

    void release(boost::shared_ptr<Connection> conn, bool status);

    /// 一些参数设置接口
    void setPoolSize(int pool_size) { pool_size_ = pool_size; }

    void setMinSize(int min_size) { min_size_ = min_size; }

    void setMaxIdleSec(int max_idle_sec) { max_idle_sec_ = max_idle_sec; }

    void setCheckIntervalSec(int check_interval_sec) { check_interval_sec_ = check_interval_sec; }

    void setServerName(std::string server_name) { server_name_ = server_name; }

    int availableCount() const;
    
    /// 获取池中服务名称
    std::string getServerName() const { return server_name_; }
    
    /// 依据服务名获取connection_pool 对象
    static boost::shared_ptr<ConnectionPool> selectConnPool(std::string server_name);

    static void addConnPool(boost::shared_ptr<ConnectionPool> conn_pool);

private:
   
    std::vector<boost::shared_ptr<Connection> >  connection_pool_;
    
    boost::mutex    mu_;

    int             pool_size_;

    int             min_size_; /// 池中保持连接状态的最少连接数

    int             max_idle_sec_;

    int             check_interval_sec_;

    int             count_;    /// 记录器，用于轮询策略

    std::string     server_name_;  /// 池中连接对应的服务名称

    int             last_checked_time_;

    // std::string     ip_;

    // int             port_;

    std::vector<std::pair<std::string, int> > ip_port_pair_;

    typedef std::unordered_map<std::string, boost::shared_ptr<ConnectionPool> > ConnectionPoolMap;

    static ConnectionPoolMap conn_pool_map_;

};





#endif
