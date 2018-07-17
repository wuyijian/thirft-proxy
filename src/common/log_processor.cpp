#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <fcntl.h>

#include "log.h"
#include "file_lister.h"
#include "regex.h"
#include "log_processor.h"

using std::string;

#define MAX_SORT_DAY 60

LogProcessor::LogProcessor(const string& workpath, int archive_days, int remove_days)
:m_workpath(workpath)
,m_archive_days(archive_days)
,m_remove_days(remove_days)
{
    DEBUG_LOG("LogProcessor contructor, workpath %s, archive_days %d, remove_days %d",
              m_workpath.c_str(), m_archive_days, m_remove_days);
};

void LogProcessor::GetFileDate(const string& filename, int& result){
    regex_t reg;
    char ebuf[128];
    const size_t nmatch = 1;
    regmatch_t rm[1];
    int ret = regcomp(&reg, "[0-9]\\{8\\}", 0); // 不支持\d以及诸多高级功能，考虑换成boost
    if (ret) {
        regerror(ret, &reg, ebuf, sizeof(ebuf));
        DEBUG_LOG("regcomp failed, %s", ebuf);
        return;
    }

    do {
        ret = regexec(&reg, filename.c_str(), nmatch, rm, 0);
        if (ret == REG_NOMATCH) {
            DEBUG_LOG("reg get no match");
            break;
        } else if (ret != 0) {
            regerror(ret, &reg, ebuf, sizeof(ebuf));
            ERROR_LOG("regexec %s failed, %s", filename.c_str(), ebuf);
            break;
        }
        
        string result_str(filename.c_str() + rm[0].rm_so, rm[0].rm_eo - rm[0].rm_so);
        DEBUG_LOG("filename is %s, rm_so %d, rm_eo %d, result is %d\n", 
                filename.c_str(), rm[0].rm_so, rm[0].rm_eo, result);
        
        result = atoi(result_str.c_str());
    } while (0);

    regfree(&reg);
}

uint32_t LogProcessor::GetTimestampByDate(int date)
{
    struct tm date_tm = {0};
    date_tm.tm_year = date / 10000 - 1900;
    date_tm.tm_mon = date / 100 % 100 - 1;
    date_tm.tm_mday = date % 100;
    return (uint32_t)mktime(&date_tm);
}

int LogProcessor::GetDayDistance(time_t from, time_t to)
{
    struct tm tm_from = {0};
    localtime_r(&from, &tm_from);
    tm_from.tm_hour= 0;
    tm_from.tm_min= 0;
    tm_from.tm_sec= 0;
    return (to - mktime(&tm_from)) / 86400;
}

void LogProcessor::SortFilesToMap()
{
    if(m_workpath.empty()){
        return;
    }
    FileLister fl;
    fl.Open(m_workpath);
    fl.Start();

    string filename;
    int date;

    time_t now = time(0);

    while(fl.Next(filename)){
        date = 0;
        /// 之前打包过的文件就不需要了哦
        /// 排除.tar.gz文件
        std::size_t found = filename.find(".tar.gz");
        if (found != std::string::npos) {
            continue;
        }
        this->GetFileDate(filename,date);
        // 简单判断
        if (date < 20000000 || date > 30000000){
            // TODO
            continue;
        }
        
        if ((GetDayDistance(GetTimestampByDate(date), now) > m_archive_days) &&
				(GetDayDistance(GetTimestampByDate(date), now) < (m_remove_days + 1))) {
            this->m_tar_files[date].insert(filename);
        }
    }

    fl.Close();
}

int LogProcessor::OpenAndWrite(const string& filename,const string& write_buf){
    int fd = open(filename.c_str(),O_CREAT|O_RDWR|O_TRUNC,S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd == -1){
        return -1;
    }

    if(write(fd,write_buf.c_str(),write_buf.size()) == -1){
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

void LogProcessor::TarMapFiles(){
    time_t now = time(0);
    struct tm tm_now = {0};
    localtime_r(&now, &tm_now);

    string day_files,tmp_file = m_workpath + "/.tmp";
    std::map<int, std::set<string> >::iterator it = this->m_tar_files.begin(),tmp_it;
    int count = 0;

    for(; it != this->m_tar_files.end();) {
        if(++count > MAX_SORT_DAY){
            break;
        }

        tmp_it = it;
        ++it;
        // string archive_name = this->get_archive_name(tm_now,tmp_it->first);
        std::stringstream ss;
        ss << tmp_it->first;
        string archive_name(ss.str());
        archive_name.append(".tar.gz");

        day_files.clear();//Initialization before use
        std::set<string>::iterator iter = tmp_it->second.begin();
        for(; iter != tmp_it->second.end(); ++iter) {
            day_files += *iter;
            day_files += '\n';
        }

        if(day_files.empty() || this->OpenAndWrite(tmp_file,day_files) != 0){
            continue;
        }

        string cmd = "cd " + m_workpath + ";" +
            "/bin/tar czf " + archive_name + " --files-from=.tmp" + " --exclude=*.tar.gz --remove-files >/dev/null 2>&1";

        DEBUG_LOG("create archive: %s", cmd.c_str());
        system(cmd.c_str());

        this->m_tar_files.erase(tmp_it);

    }
    unlink(tmp_file.c_str());
}

int LogProcessor::DoArchive()
{
    this->SortFilesToMap();
    this->TarMapFiles();

    return 0;
}

int LogProcessor::DoRemove()
{
    DIR *workdir = opendir(m_workpath.c_str());
    if(workdir == NULL)
    {
        ERROR_LOG("open %s failed: %s", m_workpath.c_str(), strerror(errno));
        return -1;
    }

    dirent* entry = NULL;
    time_t now = time(0);
    int date = 0;
    while ((entry = readdir(workdir)) != NULL) 
    {
        string filename = m_workpath + "/" + entry->d_name;

        if (entry->d_type == DT_REG) 
        {
            struct stat file_stat;
            if(stat(filename.c_str(), &file_stat) == 0)
            {
                GetFileDate(filename, date);

                // 简单判断
                if (date < 20000000 || date > 30000000){
                    // TODO
                    continue;
                }
                // if(now - file_stat.st_mtime >= time_span)
                if (GetDayDistance(GetTimestampByDate(date), now) > m_remove_days)
                {
                    if(unlink(filename.c_str()) == 0)
                        DEBUG_LOG("removed file: %s", filename.c_str());
                }
            }
        }
    }

    closedir(workdir);
    return 0;
}

void LogProcessor::Process()
{
    DoRemove();
    DoArchive();
}









