// shmem.cpp : Defines the entry point for the application.
//

#include "shmem.h"
#include <cstring>

struct SharedData {
    int counter;
    char message[256];
    bool isReady;
};

HANDLE createWorkerProcess(const char* command) {
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(
        nullptr, 
        const_cast<char*>(command), 
        nullptr, 
        nullptr, 
        FALSE, 
        0, 
        nullptr, 
        nullptr, 
        &si, 
        &pi
    )) {
        throw std::system_error(GetLastError(), std::system_category(), 
            "Failed to create worker process");
    }
    CloseHandle(pi.hThread);
    return pi.hProcess;
}

int main()
{
    try {
        // Create shared memory (main process)
        LocalSharedMemory<SharedData> shmem("Local\\MySharedMemory", true);
        
        std::cout << "Shared memory created successfully!" << std::endl;

        // Initialize with Write()
        shmem.Write([](SharedData& data) {
            data.counter = 0;
            strcpy_s(data.message, "Hello from main process!");
            data.isReady = true;
        });

        // Read with Read()
        shmem.Read([](const SharedData& data) {
            std::cout << "Counter: " << data.counter << std::endl;
            std::cout << "Message: " << data.message << std::endl;
        });

		// Create worker process that will update shared memory
		HANDLE hWorker = createWorkerProcess("worker.exe");

		// Loop until counter reaches 10 (updated by worker process)
		int iter = 0;
        while (iter < 10) {            
            shmem.Read([&iter](const SharedData& data) {
				iter = data.counter;
            });
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
