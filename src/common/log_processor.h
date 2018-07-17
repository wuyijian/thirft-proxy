#ifndef _LOG_PROCESSOR_H
#define _LOG_PROCESSOR_H

#include <string>
#include <ctime>
#include <vector>
#include <set>
#include <map>

using std::string;
using std::vector;
using std::set;
using std::string;
using std::map;

class LogProcessor
{
public:
    LogProcessor(const string& workpath, int archive_days, int remove_days);
    virtual ~LogProcessor(){}

    void GetFileDate(const string& filename, int& result);

    void SortFilesToMap();
    void TarMapFiles();
    // 生成tar包, 只是简单调用系统命令。
    int DoArchive();
    int DoRemove();

    void Process();

private:
    uint32_t GetTimestampByDate(int date);  // 获取指定日期(如20160425)起始时间戳
    int GetDayDistance(time_t from, time_t to);
    int OpenAndWrite(const string& filename,const string& write_buf);

private:
    string m_workpath;
    int m_archive_days;  // 多少天以上的文件打包
    int m_remove_days;   // 多少天以上的文件删除

    map<int, set<string> > m_tar_files;
};


#endif
