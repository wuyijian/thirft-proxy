#ifndef _CONTROL_H
#define _CONTROL_H

#include "processor.h"
#include "log_processor.h"

class Control : public IProcessor
{
public:
    virtual int Init();
    virtual int Uninit();
    // virtual int GetPkgLenCli(const char *buf, uint32_t len);
    virtual int GetPkgLenSer(int fd, const char *buf, uint32_t len);
	// virtual int CheckOpenCli(uint32_t ip, uint16_t port);
    // virtual int SelectChannel(int fd, const char *buf, uint32_t len, uint32_t ip, uint32_t work_num);
    //virtual int ShmqPushed(int fd, const char *buf, uint32_t len, int flag);
    virtual void TimeEvent();
    virtual void ProcPkgCli(int fd, const char *buf, uint32_t len);
    virtual void ProcPkgSer(int fd, const char *buf, uint32_t len);
    virtual void LinkUpCli(int fd, uint32_t ip);
    virtual void LinkUpSer(int fd, uint32_t ip, uint16_t port);
    virtual void LinkDownCli(int fd);
    virtual void LinkDownSer(int fd);

private:
    void TidyLogPath(time_t now);

private:
    //int m_archive_days;
    //int m_remove_days;
    LogProcessor *m_log_processor;
};


#endif
