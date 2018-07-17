#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <map>
#include <string>

using std::map;
using std::string;

// 非线程安全

class ConfigManager
{
public:
    ConfigManager(){
        loaded = false;
    };
    virtual ~ConfigManager(){};
    virtual bool LoadConfig(const char *source) {
        if(!_LoadConfig(source)) {
            return false;
        }
        SetLoaded();
        return true;
    };
    virtual bool ReloadConfig(const char *source) = 0;
    virtual int ConfigGetIntVal(const char *key, int defult) const = 0;
    virtual const char *ConfigGetStrVal(const char *key, const char *defult) const = 0;
	virtual void SetConfigMap(string key, string value) = 0;
    void SetLoaded() {
        loaded = true;
    }
    bool IsLoaded() {
        return loaded;
    }

protected:
    virtual bool _LoadConfig(const char *source) = 0;

protected:
    map<string, string> m_config_map;
    bool loaded;
};

#endif
