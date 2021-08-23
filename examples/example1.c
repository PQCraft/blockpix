#include <stdio.h>
#include <stdlib.h>
#include <blockpix.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int main (void) {
    srand(clock());
    #ifndef _WIN32
    signal(SIGWINCH, bp_resize);
    #endif
    bp_init();
    while (1) {
        for (int y = 0; y < bp_height; ++y) {
            for (int x = 0; x < bp_width; ++x) {
                bp_set(x, y, bp_color(x, rand() % 64, rand() % 96 - y * 8));
            }
        }
        bp_render();
    }
    bp_quit();
    return 0;
}

