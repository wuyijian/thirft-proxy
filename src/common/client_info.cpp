#include "client_info.h"


ClientInfo::ClientInfoMap ClientInfo::client_info_map;
uint32_t ClientInfo::serialized_no = 0;


uint32_t ClientInfo::GetSerializeNo() {
    return serialized_no++;
}

ClientInfo* ClientInfo::GetClientInfo(uint32_t sn) {
        if (client_info_map.find(sn) != client_info_map.end()){
            return client_info_map[sn];
        }
        return NULL;
}

uint32_t ClientInfo::SetClientInfo(ClientInfo *ci) {
        uint32_t sn = GetSerializeNo();
        ci->ser_seq = sn;
        client_info_map[sn] = ci;
        return sn;
}

void ClientInfo::RemoveClientInfo(uint32_t sn) {
        client_info_map.erase(sn);
}

void ClientInfo::run() {
    /// 客户端fd, 消息内容，以及连接池对象
    call_back_func(this, conn_pool);
}
