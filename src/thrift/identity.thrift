struct ReqProto
{
    1 : i32 userid,
}

struct AckProto
{
    1 : string identity,
}

# 将处理结果存到客服数据库，供客户进行查询

struct ReqCheckResult
{
    # 存入数据库的消息
    1 : string message, 
    2 : i32    flag, # 判定的结果 0, 通过 1，含有不文明词 2，推广或者含有个人联系方式等 3，warning，情感负面 4，疑似 
    3 : i32    appid, # 接入组件id
    4 : i32    gameid, # 游戏id,由淘米公司分配
    5 :  string nick, #昵称
    6 : i8     type, # 消息类型
    7 : i32    serverid, # 家园id/区服id
    8 : i32    userid, # 用户id
    9 : i32    rolegrade, # 角色等级
    10: i32    vipgrade, # vip等级
}


service IdentityServ
{
    AckProto GetIdentity(1: ReqProto s),

    i32      StoreMessage(1: ReqCheckResult s),
}

        
