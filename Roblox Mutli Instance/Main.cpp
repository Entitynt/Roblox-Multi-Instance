#include <windows.h>
#include <queue>
#include <thread>
#include <set>
#include "Source/Utils/Utils.h"
#include "Source/Instance Tools/InstanceTools.hpp"
#include <iostream>

bool InitializeMutex(HANDLE& Handle_Mutex) {
    Handle_Mutex = CreateMutexW(NULL, TRUE, L"ROBLOX_singletonMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Utils::WriteToConsole("[!] Mutex Already Exists!\n");
        SetConsoleTitle("Roblox Multi Instance [Mutex Already Exists]");
        return false;
    }
    else {
        Utils::WriteToConsole("[+] Mutex Started\n");
        SetConsoleTitle("Roblox Multi Instance [Mutex Started]");
        return true;
    }
}

int GetCpuLimit() {
    int cpuLimit = 0;
    char choice;
    Utils::WriteToConsole("[?] Do you want to set CPU limit? (y/n):\n");
    std::cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        Utils::WriteToConsole("[?] Enter CPU limit (1-100):\n");
        std::cin >> cpuLimit;
    }
    else {
        Utils::WriteToConsole("[#] CPU limit not set.\n");
    }
    return cpuLimit;
}

void ProcessRobloxClients(int cpuLimit, bool cpuLimiting) {
    std::queue<DWORD> injectionClients;
    std::set<DWORD> memoryProcessedProcesses;
    std::set<DWORD> cpuProcessedProcesses;

    while (true) {
        if (Utils::findRobloxClients(injectionClients)) {
            while (!injectionClients.empty()) {
                DWORD processID = injectionClients.front();
                injectionClients.pop();

                // Apply memory limit if process hasn't been processed yet
                if (memoryProcessedProcesses.find(processID) == memoryProcessedProcesses.end()) {
                    Cleaner::limitProcessMemoryUsage(processID, 250); // Set memory limit to 250 MB
                    memoryProcessedProcesses.insert(processID);
                }

                // Apply CPU limit if enabled and process hasn't been processed yet
                if (cpuLimiting && cpuProcessedProcesses.find(processID) == cpuProcessedProcesses.end()) {
                    std::this_thread::sleep_for(std::chrono::seconds(3)); // Wait for Roblox process to fully start up
                    Cleaner::LimitCpuUsage(processID, cpuLimit);
                    cpuProcessedProcesses.insert(processID);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before checking again
        memoryProcessedProcesses.clear();
    }
}

int main() {
    SetConsoleTitleA("Roblox Multi Instance");

    HANDLE Handle_Mutex;
    if (!InitializeMutex(Handle_Mutex)) {
        return 1; // Exit if mutex already exists
    }

    int cpuLimit = GetCpuLimit();
    bool cpuLimiting = (cpuLimit > 0);

    ProcessRobloxClients(cpuLimit, cpuLimiting);

    return 0;
}
