#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/move/move.hpp>
#include <boost/function.hpp>
#include <assert.h>

#include <queue>

#include <iostream>

template<typename T>

class ThreadPool : boost::noncopyable
{

public:
    

    explicit ThreadPool()
        : is_running_(false),
          queue_(),
          max_queue_size_(20)
    {}
    
    ~ThreadPool() { if (is_running_) stop(); }

    void setMaxQueueSize(int max_size) { max_queue_size_ = max_size; }

    void start(int num_threads)
    {
        is_running_ = true;
        
        for (int i = 0; i < num_threads; ++i)
        {
            thread_group_.create_thread(boost::bind(&ThreadPool::runInThread, this, i));
        }
        
    }

    void stop()
    {
        is_running_ = false;
        cond_get_.notify_all();
        thread_group_.join_all();
    }


    void put(T t) 
    {
        {
            boost::mutex::scoped_lock lock(mu_);
            while (isFull())
            {
                cond_put_.wait(mu_);
            }
            queue_.push(t);
            cond_get_.notify_one();
        }
    }

    T take()
    {
        {
            boost::mutex::scoped_lock lock(mu_);
            while(isEmpty() && is_running_)
            {
                cond_get_.wait(mu_);
            }
            T  t = NULL;
            if (!isEmpty()) 
            {
                t = queue_.front();
                queue_.pop();
                cond_put_.notify_one();
            } 
            return t;
        }
    }

    
    /// 执行从队列中取出任务，并执行里面的回调函数
    void runInThread(int i)
    {
        while(is_running_)
        {
            /// 执行逻辑
            T t = take();
            if (t != NULL) 
            {
                t->run();
            }
        }
    }


private:

    bool isFull() const { return queue_.size() == max_queue_size_; }
    bool isEmpty() const { return queue_.empty(); }
 
    bool                  is_running_;

    /// 线程池对象
    boost::thread_group   thread_group_;
    

    boost::mutex          mu_;
    
    /// 队列
    std::queue<T>        queue_;

    
    /// 等待可放入的条件变量
    boost::condition_variable_any   cond_put_;
    
    /// 等待可取出的条件变量
    boost::condition_variable_any   cond_get_;

    size_t              max_queue_size_;
};


#endif
