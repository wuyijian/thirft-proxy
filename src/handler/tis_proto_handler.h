#ifndef __TIS_PROTO_HANDLER__
#define __TIS_PROTO_HANDLER__

#include "thrift/TisService.h"
#include "thrift/IdentityServ.h"
#include "connection_pool.h"
#include "client_info.h"


void storeResult();


void tisProtoHandler(ClientInfo*, boost::shared_ptr<ConnectionPool> );



#endif
