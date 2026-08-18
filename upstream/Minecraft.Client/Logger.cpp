#include "stdafx.h"

#undef OutputDebugStringA
#undef OutputDebugStringW
#undef OutputDebugString

#include "Logger.h"
#include <dbghelp.h>
#include <time.h>
#include <intrin.h>

// stop clang from overwriting cpuid
#if defined(__cpuid)
#undef __cpuid
#endif

static FILE            *s_logFile = nullptr;
static CRITICAL_SECTION s_cs;
static bool             s_csInit  = false;

static const char *s_wittyComments[] =
{
    "Who set us up the TNT?",
    "Everything's a creeper when you think about it",
    "I bet Notch wouldn't have this problem",
    "Oops.",
    "Why did you do that?",
    "Uh... Did I do that?",
    "I just don't know what went wrong",
    "Quite honestly, I wouldn't worry myself about that.",
    "This doesn't make any sense!",
    "This is a bug, not a feature!",
    "The server is on fire.",
    "My bad.",
    "I feel sad now :(",
    "Don't be sad, have a hug! <3",
    "You should try our sister game, Minceraft!",
    "Hi. I'm Minecraft, and I'm a crashaholic.",
    "Ooh. Shiny.",
    "This is not good.",
    "Abandon ship!",
    "Oh - I know what I did wrong!",
    "Shall we play a game?",
    "Quite honestly, I wouldn't worry myself about that.",
    "Sorry, Dave.",
    "We're sorry, we're sorry, we're sorry...",
    "Never gonna give you up.",
    "A wild Herobrine appeared.",
    "Surprise! Haha. Well, this is awkward.",
    "Would you like a cupcake?",
    "Oh man. I thought that might happen.",
    "This is not a good sign.",
};

static void writeRaw(const char *msg)
{
    if (!s_logFile) return;
    fputs(msg, s_logFile);
    fflush(s_logFile);
}

static void writeLineA(const char *msg)
{
    if (!s_logFile) return;
    EnterCriticalSection(&s_cs);
    fputs(msg, s_logFile);
    fputc('\n', s_logFile);
    fflush(s_logFile);
    LeaveCriticalSection(&s_cs);
}

static void getTimestamp(char *buf, size_t size)
{
    time_t t = time(nullptr);
    struct tm tm_info;
    localtime_s(&tm_info, &t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

static void getCpuName(char *buf, size_t size)
{
    int regs[4] = {};
    char name[49] = {};
    __cpuidex(regs, 0x80000002, 0); memcpy(name,      regs, 16);
    __cpuidex(regs, 0x80000003, 0); memcpy(name + 16, regs, 16);
    __cpuidex(regs, 0x80000004, 0); memcpy(name + 32, regs, 16);
    name[48] = '\0';
    const char *p = name;
    while (*p == ' ') p++;
    strncpy_s(buf, size, p, _TRUNCATE);
}

static void getOsName(char *buf, size_t size)
{
    DWORD major = 0, minor = 0, build = 0;

    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD val = 0, sz = sizeof(val);
        if (RegQueryValueExW(hKey, L"CurrentMajorVersionNumber", nullptr, nullptr, (LPBYTE)&val, &sz) == ERROR_SUCCESS) major = val;
        sz = sizeof(val);
        if (RegQueryValueExW(hKey, L"CurrentMinorVersionNumber", nullptr, nullptr, (LPBYTE)&val, &sz) == ERROR_SUCCESS) minor = val;
        sz = sizeof(val);
        if (RegQueryValueExW(hKey, L"CurrentBuildNumber", nullptr, nullptr, (LPBYTE)&val, &sz) == ERROR_SUCCESS) build = val;
        if (build == 0)
        {
            wchar_t strBuild[16] = {};
            DWORD strSz = sizeof(strBuild);
            if (RegQueryValueExW(hKey, L"CurrentBuildNumber", nullptr, nullptr, (LPBYTE)strBuild, &strSz) == ERROR_SUCCESS)
                build = (DWORD)_wtoi(strBuild);
        }
        RegCloseKey(hKey);
    }

    SYSTEM_INFO si = {};
    GetNativeSystemInfo(&si);
    const char *arch = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "amd64" : "x86";

    snprintf(buf, size, "Windows %lu.%lu (Build %lu) (%s)", major, minor, build, arch);
}

static const char *exceptionName(DWORD code)
{
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:         return "ACCESS_VIOLATION";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_BREAKPOINT:               return "BREAKPOINT";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return "DATATYPE_MISALIGNMENT";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return "FLT_DENORMAL_OPERAND";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_FLT_INEXACT_RESULT:       return "FLT_INEXACT_RESULT";
    case EXCEPTION_FLT_INVALID_OPERATION:    return "FLT_INVALID_OPERATION";
    case EXCEPTION_FLT_OVERFLOW:             return "FLT_OVERFLOW";
    case EXCEPTION_FLT_STACK_CHECK:          return "FLT_STACK_CHECK";
    case EXCEPTION_FLT_UNDERFLOW:            return "FLT_UNDERFLOW";
    case EXCEPTION_ILLEGAL_INSTRUCTION:      return "ILLEGAL_INSTRUCTION";
    case EXCEPTION_IN_PAGE_ERROR:            return "IN_PAGE_ERROR";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "INT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_OVERFLOW:             return "INT_OVERFLOW";
    case EXCEPTION_INVALID_DISPOSITION:      return "INVALID_DISPOSITION";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "NONCONTINUABLE_EXCEPTION";
    case EXCEPTION_PRIV_INSTRUCTION:         return "PRIV_INSTRUCTION";
    case EXCEPTION_SINGLE_STEP:              return "SINGLE_STEP";
    case EXCEPTION_STACK_OVERFLOW:           return "STACK_OVERFLOW";
    default:                                 return "UNKNOWN";
    }
}

static LONG WINAPI crashHandler(EXCEPTION_POINTERS *ep)
{
    if (!s_logFile) return EXCEPTION_CONTINUE_SEARCH;

    DWORD code = ep->ExceptionRecord->ExceptionCode;
    const char *name = exceptionName(code);

    srand((unsigned)time(nullptr));
    const char *comment = s_wittyComments[rand() % (sizeof(s_wittyComments) / sizeof(s_wittyComments[0]))];

    char timestamp[64];
    getTimestamp(timestamp, sizeof(timestamp));

    char description[128];
    if (code == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2)
    {
        const char *op = ep->ExceptionRecord->ExceptionInformation[0] == 1 ? "write" : "read";
        snprintf(description, sizeof(description),
            "%s (0x%08X) - Attempted to %s from address 0x%p",
            name, code, op, (void*)ep->ExceptionRecord->ExceptionInformation[1]);
    }
    else
    {
        snprintf(description, sizeof(description), "%s (0x%08X)", name, code);
    }

    EnterCriticalSection(&s_cs);

    fprintf(s_logFile,
        "\n\n---- neoLegacy Crash Report ----\n"
        "// %s\n\n"
        "Time: %s\n"
        "Description: %s\n\n",
        comment, timestamp, description);

    HANDLE proc   = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    SymInitialize(proc, nullptr, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

    CONTEXT ctx = *ep->ContextRecord;

    STACKFRAME64 frame = {};
#ifdef _WIN64
    DWORD machineType      = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset    = ctx.Rip;
    frame.AddrStack.Offset = ctx.Rsp;
#else
    DWORD machineType      = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset    = ctx.Eip;
    frame.AddrStack.Offset = ctx.Esp;
#endif
    frame.AddrPC.Mode    = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

    char symBuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    SYMBOL_INFO *sym = (SYMBOL_INFO*)symBuf;

    IMAGEHLP_LINE64 line = {};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    fputs("Stack Trace:\n", s_logFile);

    for (int i = 0; i < 64; i++)
    {
        if (!StackWalk64(machineType, proc, thread, &frame, &ctx,
                         nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
            break;
        if (frame.AddrPC.Offset == 0) break;

        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen   = MAX_SYM_NAME;

        DWORD64 disp64 = 0;
        DWORD   disp32 = 0;

        if (SymFromAddr(proc, frame.AddrPC.Offset, &disp64, sym))
        {
            if (SymGetLineFromAddr64(proc, frame.AddrPC.Offset, &disp32, &line))
                fprintf(s_logFile, "\tat %s (%s:%lu)\n", sym->Name, line.FileName, line.LineNumber);
            else
                fprintf(s_logFile, "\tat %s+0x%llx\n", sym->Name, disp64);
        }
        else
        {
            fprintf(s_logFile, "\tat 0x%016llX\n", frame.AddrPC.Offset);
        }
    }

    SymCleanup(proc);

    fprintf(s_logFile,
        "\nA detailed walkthrough of the error, its code path and all known details is as follows:\n"
        "---------------------------------------------------------------------------\n\n");

    fprintf(s_logFile, "-- System Details --\nDetails:\n");

    fprintf(s_logFile, "\tneoLegacy Version: %s\n", NEOL_VERSION);

    char osBuf[128];
    getOsName(osBuf, sizeof(osBuf));
    fprintf(s_logFile, "\tOperating System: %s\n", osBuf);

    char cpuBuf[64];
    getCpuName(cpuBuf, sizeof(cpuBuf));
    fprintf(s_logFile, "\tCPU: %s\n", cpuBuf);

    MEMORYSTATUSEX mem = {};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    DWORDLONG freeMB  = mem.ullAvailPhys  / (1024 * 1024);
    DWORDLONG totalMB = mem.ullTotalPhys  / (1024 * 1024);
    fprintf(s_logFile, "\tMemory: %llu bytes (%llu MB) free / %llu bytes (%llu MB) total\n",
        mem.ullAvailPhys, freeMB, mem.ullTotalPhys, totalMB);

    fflush(s_logFile);
    LeaveCriticalSection(&s_cs);

    return EXCEPTION_CONTINUE_SEARCH;
}

void Logger::init()
{
    InitializeCriticalSection(&s_cs);
    s_csInit = true;

    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    char *slash = strrchr(exePath, '\\');
    if (slash) *(slash + 1) = '\0';
    strcat_s(exePath, "neoLegacy_log.txt");

    fopen_s(&s_logFile, exePath, "w");
    if (!s_logFile) return;

    SetUnhandledExceptionFilter(crashHandler);

    char timestamp[64];
    getTimestamp(timestamp, sizeof(timestamp));

    char osBuf[128];
    getOsName(osBuf, sizeof(osBuf));

    char cpuBuf[64];
    getCpuName(cpuBuf, sizeof(cpuBuf));

    fprintf(s_logFile,
        "neoLegacy log started %s\n"
        "Version: %s | OS: %s | CPU: %s\n"
        "--------------------------------------------------------------------------\n\n",
        timestamp, NEOL_VERSION, osBuf, cpuBuf);
    fflush(s_logFile);
}

void Logger::shutdown()
{
    if (s_logFile)
    {
        char timestamp[64];
        getTimestamp(timestamp, sizeof(timestamp));
        fprintf(s_logFile, "\n--------------------------------------------------------------------------\n");
        fprintf(s_logFile, "neoLegacy log ended %s\n", timestamp);
        fclose(s_logFile);
        s_logFile = nullptr;
    }
    if (s_csInit)
    {
        DeleteCriticalSection(&s_cs);
        s_csInit = false;
    }
}

void Logger::interceptA(const char *msg)
{
#ifndef _FINAL_BUILD
    OutputDebugStringA(msg);
#endif
    writeLineA(msg);
}

void Logger::interceptW(const wchar_t *msg)
{
#ifndef _FINAL_BUILD
    OutputDebugStringW(msg);
#endif
    if (!s_logFile) return;
    int needed = WideCharToMultiByte(CP_UTF8, 0, msg, -1, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return;
    char *buf = new char[needed];
    WideCharToMultiByte(CP_UTF8, 0, msg, -1, buf, needed, nullptr, nullptr);
    writeLineA(buf);
    delete[] buf;
}
