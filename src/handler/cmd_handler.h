#ifndef __CMD_HANDLER_H__
#define __CMD_HANDLER_H__
#include "client_info.h"


/// 后续会支持更多的策略,这里只是简单的依据cmd_id找处理方法的逻辑




class CmdHandler : boost::noncopyable
{
public:
    explicit CmdHandler();
    ~CmdHandler();
    
    void registerCmd(int cmd, ClientInfo::Func func);

    ClientInfo::Func getHandler(int cmd);

    void destroy();

private:
    
    /// 此map 维护命令号到处理方法的映射
    std::unordered_map<int, ClientInfo::Func> cmd_handler_map_;
};


#endif
