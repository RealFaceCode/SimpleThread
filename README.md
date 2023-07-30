# SimpleThread
Simple thread is a single header file wrapper for multithreading. The attempt of this is threading without mutex or locks.

## How to use
```cpp
#include "SimpleThread.h"

bool userWorkerThread(ThreadTask* data, uint64_t threadID)
{
    if(data->compareInstructionType("calc"))
    {
        auto& i = data->getInDataPtrAs<std::string>();
        auto& o = data->getOutDataPtrAs<std::string>();
        o += i;
        o += "thread[";
        o += std::to_string(threadID);
        o += "] ";
        o += "im filled by worker thread";
        return true;
    }
    return false;
}

int main()
{
    std::string istr("data: ");
    std::string ostr;
    ThreadTask data(&istr, &ostr, "calc", nullptr);

    ThreadPool p(4);
    p.start(userWorkerThread);

    p.addThreadTask(&data);
    bool done = false;

    while(p.checkRunning())
    {
        if(data.isFinish())
        {
            if(!done)
            {
                done = true;
                std::cout << ostr << std::endl;
                p.kill();
            }
        }
    }

    p.clean();
    return 0;
}
```

With
```cpp
#define THREAD_LOG_INFO
```
the create and destroy info for threads and threadpools can be toggled on.
