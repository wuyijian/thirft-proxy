#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <map>
#include <string>
#include "log.h"
#include "file_config_manager.h"
#include "net_utils.h"

bool FileConfigManager::ParseConfigFile(const char * file_name, map<string, string> &config_map)
{
	char pwd[1024] = {'\0'};
	getcwd(pwd, sizeof(pwd)/sizeof(char) - 1);
	Common::local_path = pwd;

    int fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        return false;
    }

    int len = lseek(fd, 0L, SEEK_END);
    lseek(fd, 0L, SEEK_SET);
    char * data = (char *)malloc(len + 1);
    if (data == NULL)
    {
        close(fd);
        return false;
    }

    bool ret = true;
    do
    {
        if (read(fd, data, len) == -1)
        {
            ret = false;
            break;
        }

        data[len] = 0;
        char * start = data;
        char * end;
        while (data + len > start)
        {
            end = strchr(start, '\n');
            if (end)
            {
                *end = 0;
            }

            if (*start != '#')
            {
                char * key;
                char * val;
                key = strtok(start, "= \t");
                val = strtok(NULL, "= \t");
                if (key != NULL && val != NULL)
                {
                    if (strncmp("include", key, 8))
                    {
                        string str_key = key;
                        string str_val = val;
                        if (config_map.find(str_key) == config_map.end())
                        {
                            config_map[str_key] = str_val;
                        }
                        else
                        {
                            BOOT_LOG(-1, "config %s same key: %s", file_name, key);
                        }

                    }
                    else
                    {
                        ret = ParseConfigFile(val, config_map);
                        if (!ret)
                        {
                            break;
                        }
                    }
                }
            }

            if (end)
            {
                start = end + 1;
            }
            else
            {
                break;
            }
        }

    }
    while (0);

    free(data);
    close(fd);

    return ret;
}

bool FileConfigManager::_LoadConfig(const char *source)
{
    m_config_map.clear();
    return ParseConfigFile(source, m_config_map);
}

bool FileConfigManager::ReloadConfig(const char * source)
{
    map<string, string> new_config_map;

    if (!ParseConfigFile(source, new_config_map)) {
        return false;
    }

    m_config_map = new_config_map;
    return true;
}

int FileConfigManager::ConfigGetIntVal(const char *key, int val) const
{
    map<string, string>::const_iterator it;
    it = m_config_map.find(key);
    if (it == m_config_map.end())
        return val;

    return atoi((*it).second.c_str());
}


const char *FileConfigManager::ConfigGetStrVal(const char *key, const char *val) const
{
    map<string, string>::const_iterator it;
    it = m_config_map.find(key);
    if (it == m_config_map.end())
        return val;

    return (*it).second.c_str();
}

void FileConfigManager::SetConfigMap(string key, string value)
{
	m_config_map[key] = value;
	return;
}
