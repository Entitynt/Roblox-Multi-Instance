#include <Windows.h>
#include "InstanceTools.hpp"
#include "../Utils/Utils.h"
#include <string>
#include <iostream>

void Cleaner::LimitCpuUsage(DWORD processId, int cpuLimitPercent) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_SET_QUOTA, FALSE, processId);
    if (hProcess == NULL) {
        Utils::WriteToConsole("[!] Process couldn't be opened. Error: " + std::to_string(GetLastError()) + "\n");
        return;
    }

    HANDLE hJob = CreateJobObject(NULL, NULL);
    if (hJob == NULL) {
        Utils::WriteToConsole("[!] Job object creation failed. Error: " + std::to_string(GetLastError()) + "\n");
        CloseHandle(hProcess);
        return;
    }

    // Set CPU limit using JOB_OBJECT_CPU_RATE_CONTROL_ENABLE and JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP
    JOBOBJECT_CPU_RATE_CONTROL_INFORMATION jobInfo = { 0 };
    jobInfo.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;
    jobInfo.CpuRate = cpuLimitPercent * 100; // Convert percentage to rate (e.g., 10% -> 1000)

    if (!SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &jobInfo, sizeof(jobInfo))) {
        Utils::WriteToConsole("[!] Failed to set CPU limit. Error: " + std::to_string(GetLastError()) + "\n");
        CloseHandle(hProcess);
        CloseHandle(hJob);
        return;
    }

    // Assign process to the job object
    if (!AssignProcessToJobObject(hJob, hProcess)) {
        Utils::WriteToConsole("[!] Failed to assign process to job. Error: " + std::to_string(GetLastError()) + "\n");
    }
    else {
        Utils::WriteToConsole("[+] CPU limit successfully applied.\n");
    }

    CloseHandle(hProcess);
    CloseHandle(hJob);
}

void Cleaner::limitProcessMemoryUsage(DWORD processID, SIZE_T memoryLimitMB) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        Utils::WriteToConsole("[!] Failed to open process for memory limiting. Error: " + std::to_string(GetLastError()) + "\n");
        return;
    }

    SIZE_T memoryLimitBytes = memoryLimitMB * 1024 * 1024;

    // Attempt to set the working set size
    if (!SetProcessWorkingSetSize(hProcess, memoryLimitBytes, memoryLimitBytes)) { //Actually I'm not sure its really useful so If it don't, don't care that
        Utils::WriteToConsole("[!] Failed to set working set size for process. Error: " + std::to_string(GetLastError()) + "\n");
    }

    // Attempt to empty the working set
    if (!EmptyWorkingSet(hProcess)) {
        Utils::WriteToConsole("[!] Failed to empty working set for process. Error: " + std::to_string(GetLastError()) + "\n");
    }

    // Check current memory usage
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        std::string memInfo = "[%] Roblox Current Memory Usage - Page File Bytes: " +
            std::to_string(pmc.PagefileUsage / (1024 * 1024)) + " MB, Working Set Bytes: " +
            std::to_string(pmc.WorkingSetSize / (1024 * 1024)) + " MB, " +
            "Process ID: " + std::to_string(processID) + "\n";
        Utils::WriteToConsole(memInfo);
    }
    else {
        Utils::WriteToConsole("[!] Failed to get memory info for process. Error: " + std::to_string(GetLastError()) + "\n");
    }

    CloseHandle(hProcess);
}
