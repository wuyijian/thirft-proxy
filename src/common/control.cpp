#include <string.h>
#include "proxy.h"
#include "async_server.h"
#include "log.h"
#include "control.h"
#include "unistd.h"

int Control::Init()
{
    string proc_name(get_proc_name());
    proc_name.append("-CONTROL");
    set_prog_name(proc_name.c_str());

    // 保证log_archive_days <= log_remove_days
    int log_archive_days = config_get_intval("log_archive_days", 7);
    log_archive_days = std::max(1, std::min(log_archive_days, 28));

    int log_remove_days = config_get_intval("log_remove_days", 28);
    log_remove_days = std::max(log_archive_days, std::min(log_remove_days, 168));
    
    char pwd[512] = {'\0'};
    getcwd(pwd, sizeof(pwd)/sizeof(char) - 1);
    string homedir = pwd;

    m_log_processor = new (std::nothrow) LogProcessor(homedir + "/log", 
            log_archive_days, log_remove_days);

    if (!m_log_processor) {
        ERROR_LOG("Control Init failed for new LogProcessor failed");
        return -1;
    }
    return 0;
}

int Control::Uninit() 
{
    return 0;
}

int Control::GetPkgLenSer(int fd, const char *buf, uint32_t len) 
{
    ERROR_LOG("Control GetPkgLenSer, should not be here");
    return 0;
}

void Control::TimeEvent() 
{
    time_t now = time(0);
    TidyLogPath(now);
}

void Control::TidyLogPath(time_t now)
{
    if(now % 86400 == 68400) // 凌晨3:00
    {
        m_log_processor->Process();
    }
}

void Control::ProcPkgCli(int fd, const char *buf, uint32_t len) 
{
    ERROR_LOG("Control ProcPkgCli, should not be here");
    return;
}

// 同步的service不会访问到这个函数，一定是异步的
void Control:: ProcPkgSer(int fd, const char *buf, uint32_t len) 
{
    ERROR_LOG("Control ProcPkgSer, should not be here");
    return;
}

void Control:: LinkUpCli(int fd, uint32_t ip) 
{
    // do nothing   
}

void Control:: LinkUpSer(int fd, uint32_t ip, uint16_t port) 
{
    ERROR_LOG("Control LinkUpSer ip %u, port %d, should not be here", 
              ip, port);
    return;
}

void Control:: LinkDownCli(int fd) 
{
    return;
}

void Control:: LinkDownSer(int fd) 
{
    ERROR_LOG("Control LinkDownSer, should not be here");
    return;
}














