// worker.cpp : Worker process that writes to shared memory
//

#include "shmem.h"
#include <cstring>
#include <thread>
#include <chrono>

struct SharedData {
    int counter;
    char message[256];
    bool isReady;
};

int main()
{
    try {
        // Open existing shared memory (worker process)
        LocalSharedMemory<SharedData> shmem("Local\\MySharedMemory", false);
        
        std::cout << "Connected to shared memory!" << std::endl;

        for (int i = 1; i <= 10; ++i) {
            shmem.Write([i](SharedData& data) {
                data.counter++;
                sprintf_s(data.message, "Worker update #%d at counter %d", 
                         i, data.counter);
                std::cout << "Wrote: Counter=" << data.counter 
                         << ", Message=\"" << data.message << "\"" << std::endl;
            });
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
