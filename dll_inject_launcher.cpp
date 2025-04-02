// trigger build by web

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

DWORD FindProcessId(const std::wstring& processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry)) {
        do {
            if (!_wcsicmp(entry.szExeFile, processName.c_str())) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}

bool InjectDLL(DWORD pid, const std::wstring& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;

    LPVOID allocMem = VirtualAllocEx(hProcess, nullptr, dllPath.size() * sizeof(wchar_t), MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(hProcess, allocMem, dllPath.c_str(), dllPath.size() * sizeof(wchar_t), nullptr);

    HMODULE hKernel32 = GetModuleHandle(L"Kernel32");
    FARPROC loadLib = GetProcAddress(hKernel32, "LoadLibraryW");

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
                                        (LPTHREAD_START_ROUTINE)loadLib,
                                        allocMem, 0, nullptr);
    if (!hThread) return false;

    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

int wmain(int argc, wchar_t* argv[]) {
    const std::wstring exePath = L"YoutubeBooster.exe";
    const std::wstring dllPath = L"BypassHook.dll";

    // 프로세스 실행
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessW(exePath.c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        std::wcerr << L"[!] YoutubeBooster.exe 실행 실패." << std::endl;
        return 1;
    }
    std::wcout << L"[*] YoutubeBooster.exe 실행됨. PID: " << pi.dwProcessId << std::endl;

    // 프로세스가 준비될 때까지 대기 (최대 5초)
    Sleep(3000);

    // DLL 주입
    if (InjectDLL(pi.dwProcessId, dllPath)) {
        std::wcout << L"[+] DLL 주입 성공!" << std::endl;
    } else {
        std::wcout << L"[!] DLL 주입 실패." << std::endl;
    }

    return 0;
}
