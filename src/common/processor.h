#ifndef _IPROCESSOR_HPP
#define _IPROCESSOR_HPP

#include <cstdlib>
#include <stdint.h>

/**
 * 业务类主接口。
 * 业务类实现该接口，以实现淘米网络程序框架
 * 与业务的隔离。
 */

struct IProcessor
{
    virtual int Init() = 0;
    virtual int Uninit() = 0;
    //virtual int GetPkgLenCli(const char *buf, uint32_t len) = 0;
    virtual int GetPkgLenSer(int fd, const char *buf, uint32_t len) = 0;
	//virtual int CheckOpenCli(uint32_t ip, uint16_t port) = 0;
    // virtual int SelectChannel(int fd, const char *buf, uint32_t len, uint32_t ip, uint32_t work_num) = 0;
    //virtual int ShmqPushed(int fd, const char *buf, uint32_t len, int flag) = 0;
    virtual void TimeEvent() = 0;
    virtual void ProcPkgCli(int fd, const char *buf, uint32_t len) = 0;
    virtual void ProcPkgSer(int fd, const char *buf, uint32_t len) = 0;
    virtual void LinkUpCli(int fd, uint32_t ip) = 0;
    virtual void LinkUpSer(int fd, uint32_t ip, uint16_t port) = 0;
    virtual void LinkDownCli(int fd) = 0;
    virtual void LinkDownSer(int fd) = 0;

    virtual ~IProcessor(){}
};

struct ISimpleProcessor
{
    virtual ~ISimpleProcessor(){}
    virtual int Init() = 0;
    virtual int Uninit() = 0;
    virtual void Process() = 0;
};

#endif 
