#include "thread_pool.h"


struct Task{
    void operator()(){
        std::cout << "hello world" << std::endl;
    }
};




int main()
{
    ThreadPool<Task> threadpool;
    
    threadpool.setMaxQueueSize(20);

    for (int i = 0; i < 10; ++i)
    {
        Task t;
        threadpool.put(t);
    }

    threadpool.start(10);
    sleep(2);
    threadpool.stop();

}
