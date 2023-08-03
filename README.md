# Simple Thread Library

Simple Thread Library is a C++ header-only library that provides a set of classes to manage and execute tasks concurrently using threads. It offers a thread-safe queue to hold tasks and a thread pool to manage the execution of those tasks.

## Features

- ThreadData: A container for data that can be shared among threads while ensuring synchronization.
- ThreadTask: Represents a task that can be executed by a thread, with input and output data containers.
- ThreadQueue: A thread-safe queue of ThreadTask objects.
- ThreadPool: Represents a pool of worker threads that can execute tasks concurrently using the ThreadQueue.

## How to Use

1. Include the "simple_thread.hpp" header in your C++ project.
2. Create a ThreadTask with the required input and output data, and a task instruction.
3. Push the ThreadTask into the ThreadQueue using ThreadPool::pushTask().
4. Start the ThreadPool with your user-defined worker function using ThreadPool::start().
5. The worker function will process the tasks concurrently in separate threads.

## Example

```cpp
#include "simple_thread.hpp"

// Example worker function for ThreadPool
bool myWorkerFunction(ThreadTask& task, uint64_t threadID) {
    // Get input data from the task
    int& inputData = task.getInData<int>();

    // Do some processing with the input data (e.g., incrementing by threadID)
    inputData += static_cast<int>(threadID);

    // Set the output data of the task (e.g., output the same value as input)
    int& outputData = task.getOutData<int>();
    outputData = inputData;

    // Simulate some work by sleeping for a random amount of time
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));

    // Return true to indicate task completion
    return true;
}

int main() {
    ThreadPool threadPool;

    // Start the ThreadPool with the worker function
    threadPool.start(myWorkerFunction);

    // Create and push some tasks to the thread pool
    int inputData = 42;
    int outputData;
    std::shared_ptr<ThreadTask> task = createThreadTask(inputData, outputData, "Task1");
    threadPool.pushTask(task);

    // You can push more tasks here...

    // Stop the ThreadPool and wait for all tasks to complete
    threadPool.stop();

    // Output the results
    std::cout << "Input Data: " << inputData << std::endl;
    std::cout << "Output Data: " << outputData << std::endl;

    return 0;
}
```

## Contributions
Contributions to the TimeHandler library are welcome. If you encounter any issues or have suggestions for improvements, please feel free to create an issue or submit a pull request on the GitHub repository.

## License
The TimeHandler library is open-source and distributed under the MIT License. See the LICENSE file for more details.
