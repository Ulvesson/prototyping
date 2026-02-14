// shmem.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <windows.h>
#include <system_error>

// Thread/Process safe local shared memory class
template<typename T>
class LocalSharedMemory {
public:
    // Constructor for creating new shared memory (main process)
    LocalSharedMemory(const std::string& name, bool create = true)
        : m_name(name)
        , m_mutexName(name + "_Mutex")
        , m_hMapFile(nullptr)
        , m_hMutex(nullptr)
        , m_pData(nullptr)
        , m_isOwner(create)
    {
        // Create or open mutex
        if (create) {
            m_hMutex = CreateMutexA(nullptr, FALSE, m_mutexName.c_str());
            if (m_hMutex == nullptr) {
                throw std::system_error(GetLastError(), std::system_category(), 
                    "Failed to create mutex");
            }
        } else {
            m_hMutex = OpenMutexA(SYNCHRONIZE, FALSE, m_mutexName.c_str());
            if (m_hMutex == nullptr) {
                throw std::system_error(GetLastError(), std::system_category(), 
                    "Failed to open mutex");
            }
        }

        // Create or open shared memory
        if (create) {
            m_hMapFile = CreateFileMappingA(
                INVALID_HANDLE_VALUE,
                nullptr,
                PAGE_READWRITE,
                0,
                sizeof(T),
                m_name.c_str()
            );
            
            if (m_hMapFile == nullptr) {
                CloseHandle(m_hMutex);
                throw std::system_error(GetLastError(), std::system_category(), 
                    "Failed to create file mapping");
            }
        } else {
            m_hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_name.c_str());
            
            if (m_hMapFile == nullptr) {
                CloseHandle(m_hMutex);
                throw std::system_error(GetLastError(), std::system_category(), 
                    "Failed to open file mapping");
            }
        }

        // Map view
        m_pData = static_cast<T*>(
            MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T))
        );

        if (m_pData == nullptr) {
            CloseHandle(m_hMapFile);
            CloseHandle(m_hMutex);
            throw std::system_error(GetLastError(), std::system_category(), 
                "Failed to map view of file");
        }

        // Initialize memory if we're the creator
        if (create) {
            Lock();
            new (m_pData) T();  // Placement new for proper initialization
            Unlock();
        }
    }

    // Destructor
    ~LocalSharedMemory() {
        if (m_pData != nullptr) {
            // Destroy the object if we're the owner
            if (m_isOwner) {
                Lock();
                m_pData->~T();
                Unlock();
            }
            UnmapViewOfFile(m_pData);
        }
        if (m_hMapFile != nullptr) {
            CloseHandle(m_hMapFile);
        }
        if (m_hMutex != nullptr) {
            CloseHandle(m_hMutex);
        }
    }

    // Delete copy constructor and assignment operator
    LocalSharedMemory(const LocalSharedMemory&) = delete;
    LocalSharedMemory& operator=(const LocalSharedMemory&) = delete;

    // Move constructor and assignment operator
    LocalSharedMemory(LocalSharedMemory&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_mutexName(std::move(other.m_mutexName))
        , m_hMapFile(other.m_hMapFile)
        , m_hMutex(other.m_hMutex)
        , m_pData(other.m_pData)
        , m_isOwner(other.m_isOwner)
    {
        other.m_hMapFile = nullptr;
        other.m_hMutex = nullptr;
        other.m_pData = nullptr;
        other.m_isOwner = false;
    }

    LocalSharedMemory& operator=(LocalSharedMemory&& other) noexcept {
        if (this != &other) {
            // Clean up existing resources
            this->~LocalSharedMemory();
            
            // Move resources
            m_name = std::move(other.m_name);
            m_mutexName = std::move(other.m_mutexName);
            m_hMapFile = other.m_hMapFile;
            m_hMutex = other.m_hMutex;
            m_pData = other.m_pData;
            m_isOwner = other.m_isOwner;
            
            other.m_hMapFile = nullptr;
            other.m_hMutex = nullptr;
            other.m_pData = nullptr;
            other.m_isOwner = false;
        }
        return *this;
    }

    // Execute a function with exclusive access to shared memory
    template<typename Func>
    auto WithLock(Func&& func) -> decltype(func(std::declval<T&>())) {
        Lock();
        try {
            if constexpr (std::is_void_v<decltype(func(*m_pData))>) {
                func(*m_pData);
                Unlock();
            } else {
                auto result = func(*m_pData);
                Unlock();
                return result;
            }
        } catch (...) {
            Unlock();
            throw;
        }
    }

    // Read-only access with lock
    template<typename Func>
    auto Read(Func&& func) const -> decltype(func(std::declval<const T&>())) {
        Lock();
        try {
            if constexpr (std::is_void_v<decltype(func(*m_pData))>) {
                func(*m_pData);
                Unlock();
            } else {
                auto result = func(*m_pData);
                Unlock();
                return result;
            }
        } catch (...) {
            Unlock();
            throw;
        }
    }

    // Write access with lock
    template<typename Func>
    void Write(Func&& func) {
        WithLock(std::forward<Func>(func));
    }

    // Manual lock/unlock (use with caution - prefer WithLock/Read/Write)
    void Lock() const {
        DWORD result = WaitForSingleObject(m_hMutex, INFINITE);
        if (result != WAIT_OBJECT_0) {
            throw std::system_error(GetLastError(), std::system_category(), 
                "Failed to acquire mutex");
        }
    }

    void Unlock() const {
        if (!ReleaseMutex(m_hMutex)) {
            throw std::system_error(GetLastError(), std::system_category(), 
                "Failed to release mutex");
        }
    }

    // Check if this instance created the shared memory
    bool IsOwner() const { return m_isOwner; }

    // Get the name of the shared memory
    const std::string& GetName() const { return m_name; }

private:
    std::string m_name;
    std::string m_mutexName;
    HANDLE m_hMapFile;
    HANDLE m_hMutex;
    T* m_pData;
    bool m_isOwner;
};
