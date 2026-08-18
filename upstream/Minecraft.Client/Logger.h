#pragma once

#define NEOL_VERSION "TU43"

class Logger
{
public:
    static void init();
    static void shutdown();
    static void interceptA(const char *msg);
    static void interceptW(const wchar_t *msg);
};
