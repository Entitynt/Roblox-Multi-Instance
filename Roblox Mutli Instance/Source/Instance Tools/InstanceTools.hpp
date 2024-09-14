#pragma once

class Cleaner
{
public:
	static void limitProcessMemoryUsage(DWORD processID, SIZE_T memoryLimitMB);
	static void LimitCpuUsage(DWORD processId, int cpuLimitPercent);
};