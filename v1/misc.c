#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <unistd.h>
#include "defs.h"

int getTimeMs() {
    return GetTickCount();
}

int inputWaiting() {
    fd_set readfds;
    struct timeval tv;
    FD_ZERO (&readfds);
    FD_SET (fileno(stdin), &readfds);
    tv.tv_sec = 0; tv.tv_usec = 0;
    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
}

void readInput(S_SEARCHINFO *info) {
    int bytes;
    char input[256] = "", *endc;

    if (inputWaiting()) {
        info -> stopped = true;
        
        do {
            bytes = read(fileno(stdin), input, 256);
        } while (bytes < 0);

        endc = strchr(input,'\n');
        if (endc) *endc = 0;

        if (strlen(input) > 0) {
            if (!strncmp(input, "quit", 4)) {
                info -> quit = true;
            }

            else if (!strncmp(input, "stop", 4))
                info -> quit = true;
        }
    }
}