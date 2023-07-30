#ifndef SIMPLETHREAD_H
#define SIMPLETHREAD_H

#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <vector>

#define hash(str) std::hash<std::string>{}(str)

struct ThreadTask
        {
        private:
            bool finish = false;
            bool failed = false;
            bool working = false;
            void* inData = nullptr;
            void* outData = nullptr;
            ThreadTask* depends = nullptr;
            uint64_t instructionType = 0;

        public:
            ThreadTask() = default;
            ThreadTask(void* inData, void* outData, const std::string& iType, ThreadTask* dData)
            : inData(inData), outData(outData), instructionType(hash(iType)), depends(dData)
            {}

            void setInDataPtr(void* data)
            {
                this->inData = data;
            }

            void setOutDataPtr(void* data)
            {
                this->outData = data;
            }

            template<typename RTYPE>
            RTYPE& getInDataPtrAs()
            {
                return *(RTYPE*)inData;
            }

            template<typename RTYPE>
            RTYPE& getOutDataPtrAs()
            {
                return *(RTYPE*)outData;
            }

            void setInstructionType(const std::string& iType)
            {
                instructionType = hash(iType);
            }

            bool compareInstructionType(const std::string& iType) const
            {
                return instructionType == hash(iType);
            }

            void setDependency(ThreadTask* dData)
            {
                depends = dData;
            }

            bool isDependencyReady()
            {
                if(!depends)
                    return true;
                if(depends->finish)
                    return true;
                return false;
            }

            bool isFinish() const
            {
                return finish;
            }

            void setFinish(const bool& f = true)
            {
                finish = f;
            }

            bool hasFailed() const
            {
                return failed;
            }

            void setFailed()
            {
                failed = true;
            }

            bool isWorking() const
            {
                return working;
            }

            void setWorking(const bool& work)
            {
                working = work;
            }
        };

enum ThreadStatus
        {
    Offline,
    Idle,
    Working,
    };

void workerFunc(std::deque<ThreadTask*>* iData, ThreadStatus* stat, bool* running, bool* pullFromQueue, uint64_t threadID, bool uWorkerFunc(ThreadTask*, uint64_t))
{
    while(*running)
    {
        if(*stat == Offline)
            break;

        ThreadTask* data = nullptr;

        if(!*pullFromQueue)
        {
            *pullFromQueue = true;
            if(iData->empty())
            {
                *stat = Idle;
                continue;
            }

            data = iData->front();

            if(data->isWorking())
                continue;
            if(!data->isDependencyReady())
                continue;

            data->setWorking(true);

            iData->pop_front();
            *pullFromQueue = false;
        }

        if(!data)
        {
            *stat = Idle;
            continue;
        }

        if(data->isFinish())
        {
            *stat = Idle;
            continue;
        }

        *stat = Working;
        data->setFinish(uWorkerFunc(data, threadID));
        data->setWorking(false);

        if(!data->isFinish())
            data->setFailed();
    }
    *stat = Offline;
}

struct Thread
{
private:
    std::thread thread;
    ThreadStatus wStat = Offline;
    bool running = false;
    uint64_t id;
    static uint64_t threadID;

public:
    Thread() = default;

    Thread(bool uWorkerFunc(ThreadTask*, uint64_t), std::deque<ThreadTask*>* data, bool* pullFromQueue)
    {
        run(uWorkerFunc, data, pullFromQueue);
    }

    void run(bool uWorkerFunc(ThreadTask*, uint64_t), std::deque<ThreadTask*>* data, bool* pullFromQueue)
    {
        id = threadID;
        threadID++;
        wStat = Working;
        running = true;
        thread = std::thread(workerFunc, data, &wStat, &running, pullFromQueue, id, uWorkerFunc);
        thread.detach();
#ifdef THREAD_LOG_INFO
        {
            std::string str("Started thread with ID[");
            str += std::to_string(id);
            str += "]";
            std::cout << str << std::endl;
        }
#endif
    }

    void kill()
    {
        running = false;
        pthread_kill(thread.native_handle(),0);
#ifdef THREAD_LOG_INFO
        {
            std::string str("Killed thread with ID[");
            str += std::to_string(id);
            str += "]";
            std::cout << str << std::endl;
        }
#endif
    }

    ThreadStatus getThreadStatus() const
    {
        return wStat;
    }

    uint64_t getThreadID() const
    {
        return threadID;
    }

    bool isRunning()
    {
        return running;
    }
};
uint64_t Thread::threadID = 0;

struct ThreadPool
{
private:
    std::deque<ThreadTask*> data;
    bool pullingFromQueue = false;
    std::vector<Thread*> threads;
    uint8_t maxNumThreads;

public:
    ThreadPool()
    {}

    ThreadPool(const uint8_t& numThreads)
    {
        uint8_t numCores = std::thread::hardware_concurrency();
        if(numThreads > numCores - 1)
            maxNumThreads = numCores;
        maxNumThreads = numThreads;
    }

    void start(bool uFunc(ThreadTask*, uint64_t))
    {
#ifdef THREAD_LOG_INFO
        std::cout << "Started thread pool with " << (int)maxNumThreads << " threads" << std::endl;
#endif
        for(int i = 0; i < maxNumThreads; i++)
            threads.emplace_back(new Thread(uFunc, &data, &pullingFromQueue));
    }

    void kill()
    {
        for(auto& t : threads)
            t->kill();
#ifdef THREAD_LOG_INFO
        std::cout << "Killed thread pool with " << (int)maxNumThreads << " threads" << std::endl;
#endif
    }

    void addThreadTask(ThreadTask* task)
    {
        data.emplace_back(task);
    }

    bool checkRunning()
    {
        for(auto& t : threads)
            if(t->getThreadStatus() != Offline)
                return true;
            return false;
    }

    void clean()
    {
        if(checkRunning())
            return;
        for(auto* t : threads)
            delete t;
        threads.clear();
    }};
#endif //SIMPLETHREAD_H
