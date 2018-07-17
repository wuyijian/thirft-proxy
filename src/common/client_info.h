#ifndef _CLIENT_INFO_H
#define _CLIENT_INFO_H

#include <unordered_map>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "list.h"
#include "timer.h"

#include "../connection/connection_pool.h"

class ClientInfo
{
public:

    static uint32_t GetSerializeNo();
    static ClientInfo* GetClientInfo(uint32_t sn);
    static uint32_t SetClientInfo(ClientInfo *ci);
    static void RemoveClientInfo(uint32_t sn);

public:
    int              cli_fd;
    uint32_t         cli_seq;
    uint16_t         cli_cmd;
    uint32_t         userid;
    uint32_t         ser_seq;

    uint32_t         len;

    struct timeval   proc_time;

    list_head_t      fd_list_node;
    timer_event_t    event;

    
    boost::shared_ptr<ConnectionPool> conn_pool;

    char buf[4096];
    
    /// 注册消息处理函数
    typedef boost::function<void(ClientInfo *, boost::shared_ptr<ConnectionPool>)> Func;
    Func            call_back_func;
    
    template<typename CallBack>
    void accept(CallBack cb) { call_back_func = cb; }

    /// 此函数会调用回调函数
    void run();    
    typedef std::unordered_map<uint32_t, ClientInfo*> ClientInfoMap;
    typedef std::unordered_map<uint32_t, ClientInfo*>::iterator ClientInfoMapIter;
    static  ClientInfoMap client_info_map;

    static uint32_t serialized_no;
    // static boost::mutex     mu;
};

#endif
