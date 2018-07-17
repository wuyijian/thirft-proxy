#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <boost/noncopyable.hpp>
#include <boost/move/move.hpp>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <string>

#include "log.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;



class Connection : boost::noncopyable
{
public:
    explicit Connection(std::string ip, int port)
        : ip_(ip),
          port_(port),
          is_busy_(false),
          is_available_(false),
          is_dead_(true)
    {}

    ~Connection() { destroy(); }
    void init()
    {
        try 
        {
            socket_ = boost::make_shared<TSocket>(ip_, port_);
            
            socket_->setConnTimeout(500);
            socket_->setRecvTimeout(10000);
            socket_->setSendTimeout(10000);

            transport_ = boost::make_shared<TFramedTransport>(socket_);

            transport_->open();
            is_dead_ = false;
        } catch (TException & tx) {
            is_dead_ = true;
        }
    }
        
    /// close 一般一会有问题，不处理异常
    void destroy() { transport_->close(); }
    
    bool isBusy() const { return is_busy_; }

    bool isAvailable() const { return !is_busy_ && !is_dead_ && transport_->isOpen(); }

    bool isDead() const { return is_dead_; }
    
    void setBusy(bool is_busy) { is_busy_  = is_busy; }

    void setAvailable(bool is_available) { is_available_  = is_available; }

    void setDead(bool is_dead) { is_dead_ = is_dead; }

    void setLastUsedTime(int last_used_time) { last_used_time_ = last_used_time; }

    int getLastUsedTime() { return last_used_time_; }

    boost::shared_ptr<TTransport> getTransport() const { return transport_; }
    
    void release(bool state) 
    {
        if (state)
        {
            setDead(false);
        } else {
            setDead(true);
        }
        /// 最后设置这个状态
        setBusy(false);
    }

private:
    std::string ip_;

    int         port_;

    /// 上次使用时间
    int         last_used_time_;

    /// 是否在使用中
    bool        is_busy_;

    /// 是否可用
    bool        is_available_;

    /// 死亡
    bool        is_dead_;


    boost::shared_ptr<TSocket> socket_;
    boost::shared_ptr<TTransport> transport_;

};
#endif
