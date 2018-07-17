#include "cmd_handler.h"
#include "log.h"
#include "proxy.h"
#include "default_proto_handler.h"



CmdHandler::CmdHandler(){}

CmdHandler::~CmdHandler()
{
    destroy();
}



/// 注册命令处理函数
void CmdHandler::registerCmd(int cmd, ClientInfo::Func fun)
{
    cmd_handler_map_.insert(std::make_pair(cmd, fun));
}

/// 处理协议
ClientInfo::Func CmdHandler::getHandler(int cmd)
{
    if (cmd_handler_map_.count(cmd) == 0)
    {
        ERROR_LOG("invalied cmd, cmd is %d" , cmd);
        return ClientInfo::Func(defaultProtoHandler);
    } else {
        if (cmd_handler_map_[cmd]) {
            return cmd_handler_map_[cmd];
        }
        ERROR_LOG("invalied handler, cmd is %d", cmd);
        return ClientInfo::Func(defaultProtoHandler); 
    }
}


void CmdHandler::destroy()
{
    cmd_handler_map_.clear();
}

/// TODO del function
