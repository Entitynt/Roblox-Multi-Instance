#include <windows.h>
#include <queue>
#include <Psapi.h>

class Utils
{
public:
    static void SetColor(WORD color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }

    static void WriteToConsole(const std::string& text) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written;
        WriteConsoleA(hConsole, text.c_str(), (DWORD)text.size(), &written, NULL);
        SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  // Reset to default color
    }

    // Find all instances of RobloxPlayerBeta.exe and push their process IDs into a queue
    static bool findRobloxClients(std::queue<DWORD>& injection_clients) {
        DWORD processIDs[1024], cbNeeded;

        if (!EnumProcesses(processIDs, sizeof(processIDs), &cbNeeded)) {
            return false;
        }

        const DWORD processCount = cbNeeded / sizeof(DWORD);

        for (DWORD i = 0; i < processCount; ++i) {
            if (processIDs[i] == 0) continue;

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIDs[i]);
            if (hProcess) {
                HMODULE hMod;
                DWORD cbNeededMod;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
                    char processName[MAX_PATH];
                    if (GetModuleBaseNameA(hProcess, hMod, processName, sizeof(processName))) {
                        if (_stricmp(processName, "RobloxPlayerBeta.exe") == 0) {
                            injection_clients.push(processIDs[i]);
                        }
                    }
                }
                CloseHandle(hProcess);
            }
        }

        return !injection_clients.empty();
    }
};