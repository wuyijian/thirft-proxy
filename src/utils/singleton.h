#ifndef  _SINGLETON_H_ 
#define  _SINGLETON_H_ 

template<typename T> 
class Singleton 
{
public: 
    static T* &GetInstance() 
    {
        if (!instance_) {
            instance_ = new (std::nothrow) T();
        }
        
        return instance_; 
    } 
private: 
    static T *instance_; 
};

template<typename T> 
T *Singleton<T>::instance_ = NULL;

#endif   // _SINGLETON_H_
