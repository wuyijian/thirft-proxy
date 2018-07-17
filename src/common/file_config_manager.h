#ifndef FILE_CONFIG_MANAGER_H
#define FILE_CONFIG_MANAGER_H

#include "config_manager.h"

// 非线程安全

class FileConfigManager : public ConfigManager
{
public:
    virtual bool _LoadConfig(const char *source);
    virtual bool ReloadConfig(const char *source);
    virtual int ConfigGetIntVal(const char *key, int defult) const;
    virtual const char *ConfigGetStrVal(const char *key, const char *defult) const;
	virtual void SetConfigMap(string key, string value);

protected:
    bool ParseConfigFile(const char *filename, map<string, string> &config_map);

private:
    //map<string, string> m_config_map;
};

#endif
