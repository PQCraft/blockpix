#include <stdio.h>
#include <stdlib.h>
#include <blockpix.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

int main (void) {
    srand(clock());
    signal(SIGWINCH, bp_resize);
    bp_init();
    while (5) {
        for (int y = 0; y < bp_height; ++y) {
            for (int x = 0; x < bp_width; ++x) {
                bp_set(x, y, bp_color(x, rand() % 64, rand() % 96 - y * 8));
            }
        }
        bp_render();
        //sleep(1);
    }
    bp_quit();
    return 0;
}

