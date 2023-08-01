#include <memory>
#include <thread>
#include <queue>
#include <iostream>
#include <chrono>
#include <functional>
#include <random>

/**
 * @class ThreadData
 * @brief Represents a container for data that can be shared among threads.
 *
 * The ThreadData class ensures that only one thread can access the data at a time through synchronization.
 */
class ThreadData
        {
        private:
            std::shared_ptr<void*> mData;
            bool mWorking;
        private:
            /**
            * @brief Waits until the data is free (not being worked on by other threads).
            */
            void p_waitUntilFree()
            {
                while(mWorking);
            }

        public:
            /**
            * @brief Default constructor.
            * Initializes the ThreadData object with no data and sets it as not being worked on.
            */
            ThreadData()
            : mData(nullptr), mWorking(false)
            {}

            /**
             * @brief Constructor with data input.
             * Initializes the ThreadData object with the provided data and sets it as not being worked on.
             *
             * @tparam IN_DATA The type of input data.
             * @param[in] data The input data.
             */
            template<typename IN_DATA>
            ThreadData(IN_DATA& data)
            : mData(std::make_shared<void*>(static_cast<void*>(&data))), mWorking(false)
            {}

            /**
             * @brief Retrieves the shared pointer to the data after waiting until the data is free.
             *
             * @return Shared pointer to the data.
             */
            std::shared_ptr<void*> getData()
            {
                p_waitUntilFree();
                mWorking = true;
                return mData;
            }

            /**
             * @brief Unlocks the data after the operation is completed, indicating that the data is free for other threads to access.
             */
            void unlock()
            {
                mWorking = false;
            }
        };

/**
 * @class ThreadTask
 * @brief Represents a task that can be executed by a thread.
 *
 * The ThreadTask class contains input and output ThreadData objects, indicating the data that the task requires as input and produces as output.
 * It also manages task completion status and provides a way to compare tasks based on their instructions.
 */
class ThreadTask
        {
        private:
            ThreadData mIn;
            ThreadData mOut;
            bool mFinished;
            uint64_t mTaskInstruction;

        public:
            /**
             * @brief Default constructor.
             * Initializes the ThreadTask object with no data and sets it as not finished.
             */
            ThreadTask()
            : mFinished(false), mTaskInstruction(0)
            {}

            /**
             * @brief Constructor with input and output data and a task instruction.
             * Initializes the ThreadTask object with the provided input and output data, sets it as not finished, and computes a hash value for the given task instruction.
             *
             * @tparam IN_DATA The type of input data.
             * @tparam OUT_DATA The type of output data.
             * @param[in] inData The input data.
             * @param[in] outData The output data.
             * @param[in] taskInstruction The instruction associated with the task.
             */
            template<typename IN_DATA, typename OUT_DATA>
            ThreadTask(IN_DATA& inData, OUT_DATA& outData, const std::string& taskInstruction)
            : mIn(inData),
            mOut(outData),
            mFinished(false),
            mTaskInstruction(std::hash<std::string>()(taskInstruction))
            {}

            /**
             * @brief Retrieves the input data of the specified type.
             *
             * @tparam TYPE The type of input data.
             * @return Reference to the input data of the specified type.
             */
            template<typename TYPE>
            TYPE& getInData()
            {
                return *static_cast<TYPE*>(*mIn.getData());
            }

            /**
             * @brief Retrieves the output data of the specified type.
             *
             * @tparam TYPE The type of output data.
             * @return Reference to the output data of the specified type.
             */
            template<typename TYPE>
            TYPE& getOutData()
            {
                return *static_cast<TYPE*>(*mOut.getData());
            }

            /**
             * @brief Unlocks the input data after the operation is completed, indicating that it is free for other threads to access.
             */
            void unlockInData()
            {
                mIn.unlock();
            }

            /**
             * @brief Unlocks the output data after the operation is completed, indicating that it is free for other threads to access.
             */
            void unlockOutData()
            {
                mOut.unlock();
            }

            /**
             * @brief Checks whether the task has finished its execution.
             *
             * @return True if the task has finished; otherwise, false.
             */
            bool isFinished() const
            {
                return mFinished;
            }

            /**
             * @brief Marks the task as finished.
             */
            void setFinish()
            {
                mFinished = true;
            }

            /**
             * @brief Compares the task's instruction with the given instruction and returns true if they match, otherwise false.
             *
             * @param[in] taskInstruction The instruction to compare with the task's instruction.
             * @return True if the instructions match; otherwise, false.
             */
            bool compareTaskInstruction(const std::string& taskInstruction)
            {
                return mTaskInstruction == std::hash<std::string>()(taskInstruction);
            }
        };

/**
 * @brief Creates a new instance of the ThreadTask class with the provided input and output data and a task instruction.
 *
 * @tparam IN_DATA The type of input data.
 * @tparam OUT_DATA The type of output data.
 * @param[in] inData A reference to the input data that the task requires.
 * @param[in] outData A reference to the output data that the task will produce.
 * @param[in] taskInstruction The instruction associated with the task.
 * @return A shared pointer to the newly created ThreadTask object.
 */
template<typename IN_DATA, typename OUT_DATA>
std::shared_ptr<ThreadTask> createThreadTask(IN_DATA& inData, OUT_DATA& outData, const std::string& taskInstruction)
{
    return std::make_shared<ThreadTask>(inData, outData, taskInstruction);
}

/**
 * @class ThreadQueue
 * @brief Represents a thread-safe queue of ThreadTask objects.
 *
 * The ThreadQueue class allows tasks to be pushed into the queue and retrieved in a thread-safe manner.
 */
class ThreadQueue
        {
        private:
            std::deque<std::shared_ptr<ThreadTask>> mTasks;
            bool mWorkOnQueue;

        private:
            /**
             * @brief Waits until the queue is free to avoid concurrent access.
             */
            void p_waitUntilFree() const
            {
                while(mWorkOnQueue);
            }

        public:
            /**
             * @brief Default constructor.
             * Initializes the ThreadQueue object with an empty task queue and sets it as not being worked on.
             */
            ThreadQueue()
            : mTasks(), mWorkOnQueue(false)
            {}

            /**
             * @brief Destructor.
             */
            ~ThreadQueue()
            {}

            /**
             * @brief Pushes a task into the queue, waiting until the queue is free to avoid concurrent access.
             *
             * @param[in] task A shared pointer to the task to be added to the queue.
             */
            void pushTask(std::shared_ptr<ThreadTask>& task)
            {
                p_waitUntilFree();
                mWorkOnQueue = true;
                mTasks.emplace_back(task);
                mWorkOnQueue = false;
            }

            /**
             * @brief Retrieves the first task from the queue if available, waiting until the queue is free to avoid concurrent access.
             *
             * @return A shared pointer to the first task in the queue, or nullptr if the queue is empty.
             */
            std::shared_ptr<ThreadTask> getFirstTask()
            {
                if (!mTasks.empty())
                {
                    p_waitUntilFree();
                    mWorkOnQueue =  true;
                    auto r = mTasks.front();
                    mWorkOnQueue = false;
                    return r;
                }
                else
                    return nullptr;
            }

            /**
             * @brief Removes the first task from the queue if available, waiting until the queue is free to avoid concurrent access.
             */
            void removeFirstTask()
            {
                if (!mTasks.empty())
                {
                    p_waitUntilFree();
                    mWorkOnQueue =  true;
                    mTasks.pop_front();
                    mWorkOnQueue = false;
                }
            }
        };

/**
 * @class ThreadPool
 * @brief Represents a pool of worker threads that can execute tasks concurrently.
 *
 * The ThreadPool class uses a ThreadQueue to manage the tasks for the worker threads.
 */
class ThreadPool
        {
        private:
            std::vector<std::thread> mThreads;
            ThreadQueue mTaskQueue;
            bool mStop;
            int mMaxThreads;

        public:
            /**
             * @brief Default constructor.
             * Initializes the ThreadPool object with the maximum number of threads allowed based on the available hardware cores.
             */
            ThreadPool()
            : mStop(false)
            {
                int numCores = std::thread::hardware_concurrency();
                mMaxThreads = std::max(1, numCores - 1);
            }

            /**
             * @brief Constructor with a specified maximum number of threads.
             * Initializes the ThreadPool object with the given maximum number of threads, which should be less than or equal to the available hardware cores.
             *
             * @param[in] maxThreads The maximum number of threads in the thread pool.
             */
            ThreadPool(int maxThreads)
            : mStop(false), mMaxThreads(maxThreads)
            {
                int numCores = std::thread::hardware_concurrency();
                mMaxThreads = std::min(maxThreads, std::max(1, numCores - 1));
            }

            /**
             * @brief Starts the thread pool with the given user-defined worker function that will be executed by the worker threads.
             *
             * @param[in] userWorkerFunc The user-defined worker function to be executed by the worker threads.
             */
            void start(std::function<bool(ThreadTask&, uint64_t)> userWorkerFunc)
            {
                for (int i = 0; i < mMaxThreads; ++i)
                {
                    mThreads.emplace_back(std::bind(&ThreadPool::workerFunc, this, userWorkerFunc, i));
                }
            }


            /**
             * @brief Stops the thread pool and waits for all worker threads to finish their tasks before cleaning up.
             */
            void stop()
            {
                mStop = true;
                for (auto& thread : mThreads)
                {
                    if (thread.joinable())
                        thread.join();
                }
                mThreads.clear();
                mStop = false;
            }


            /**
             * @brief Pushes a task into the task queue of the thread pool.
             *
             * @param[in] task A shared pointer to the task to be added to the task queue.
             */
            void pushTask(std::shared_ptr<ThreadTask> task)
            {
                mTaskQueue.pushTask(task);
            }


            void workerFunc(std::function<bool(ThreadTask&, uint64_t)> userWorkerFunc, uint64_t threadID)
            {
                while (!mStop)
                {
                    std::shared_ptr<ThreadTask> task = mTaskQueue.getFirstTask();
                    if (task)
                    {
                        if (userWorkerFunc(*task, threadID))
                        {
                            task->setFinish();
                        }
                        else
                        {
                            task->unlockInData();
                        }
                        task->unlockOutData();
                        mTaskQueue.removeFirstTask();
                    }
                    else
                        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                }
            }

            /**
             * @brief Returns the number of active threads in the thread pool.
             *
             * @return The number of active threads in the thread pool.
             */
            int getNumActiveThreads() const
            {
                int numActiveThreads = 0;
                for (const auto& thread : mThreads)
                {
                    if (thread.joinable())
                        numActiveThreads++;
                }
                return numActiveThreads;
            }

            /**
             * @brief Returns the maximum number of threads allowed in the thread pool.
             *
             * @return The maximum number of threads allowed in the thread pool.
             */
            int getMaxThreads() const
            {
                return mMaxThreads;
            }

            /**
             * @brief Adds additional threads to the thread pool up to the specified number, considering the available hardware cores and the current number of threads.
             *
             * @param[in] numThreads The number of threads to add to the thread pool.
             * @param[in] userWorkerFunc The user-defined worker function to be executed by the newly added threads.
             */
            void addThreads(int numThreads, std::function<bool(ThreadTask&, uint64_t)> userWorkerFunc)
            {
                int numCores = std::thread::hardware_concurrency();
                int maxAdditionalThreads = std::max(0, numCores - 1 - static_cast<int>(mThreads.size()));
                int numThreadsToAdd = std::min(numThreads, maxAdditionalThreads);
                mMaxThreads += numThreadsToAdd;

                for (int i = 0; i < numThreadsToAdd; ++i)
                {
                    mThreads.emplace_back(std::bind(&ThreadPool::workerFunc, this, userWorkerFunc, i));
                }
            }
        };
