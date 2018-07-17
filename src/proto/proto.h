#ifndef _PROTO_H
#define _PROTO_H

struct ProtoHeader
{
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
}__attribute__((packed));

struct ProtoBody
{
    char     identity[64];
}__attribute__((packed));

struct Content
{
    char     identity1[64];
    char     identity2[64];
}__attribute__((packed));


#define WRITE_MESSAGE_CMD_ID 0x4702
struct ContentBody
{
    uint32_t    content_len;
    char        content[];
}__attribute__((packed));

/*小花仙*/
struct DataBodyHuaXian
{
    uint32_t    game_id;
    char        nick[16];
    uint8_t     type;
    uint32_t    home_id;
    uint32_t    role_grade;
    uint32_t    vip_grade;
}__attribute__((packed));


/*花语学园*/
struct DataBodyHuaYu
{
    uint32_t    game_id;
    char        nick[16];
    uint8_t     type;
    uint32_t    home_id;
    uint32_t    role_grade;
    uint32_t    vip_grade;
    uint32_t    msg_type;// 消息类型 0:文本 1:语音
}__attribute__((packed));

struct ResBody
{
    uint32_t dirty_flag;
}__attribute__((packed));








#endif
