#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#endif

#include "util.h"

int get_time_ms()
{
#ifdef WIN32
    return GetTickCount();
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
#endif
}


// http://home.arcor.de/dreamlike/chess/
int InputWaiting()
{
#ifndef WIN32
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    tv.tv_sec = 0; tv.tv_usec = 0;
    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
#else
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }
    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
            return 1;
        return dw;
    }
    else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
#endif
}

void read_input(SearchInfo *info)
{
    // At EOF stdin stays readable forever, so stop polling instead of killing the search
    static bool stdin_eof = false;
    char input[256] = "", *endc;

    if (stdin_eof || !InputWaiting())
        return;

    int bytes = read(fileno(stdin), input, sizeof(input) - 1);
    if (bytes == 0)
        stdin_eof = true;
    if (bytes <= 0)
        return;

    input[bytes] = 0;
    endc = strchr(input, '\n');
    if (endc)
        *endc = 0;

    // Only stop and quit end the search; other commands must not disturb it
    if (!strncmp(input, "quit", 4))
        exit(0);
    else if (!strncmp(input, "stop", 4))
        info->stopped = true;
    else if (!strncmp(input, "isready", 7))
        printf("readyok\n");
}
