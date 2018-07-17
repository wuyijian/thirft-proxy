#ifndef __DEFAULT_PROTO_HANDLER_H__
#define __DEFAULT_PROTO_HANDLER_H__

#include "connection_pool.h"
#include "client_info.h"
#include "proxy.h"

void defaultProtoHandler(ClientInfo* ci, boost::shared_ptr<ConnectionPool> )
{
    Proxy::SendToClient(ci, NULL, 0, 1001);
}





#endif
