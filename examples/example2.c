#include <stdio.h>
#include <stdlib.h>
#include <blockpix.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

void wait_us(uint64_t d) {
    struct timespec dts;
    dts.tv_sec = d / 1000000;
    dts.tv_nsec = (d % 1000000) * 1000;
    nanosleep(&dts, NULL);
}

void clean_quit() {
    bp_quit();
    printf("Exiting...\n");
    exit(0);
}

int main (void) {
    srand(clock());
    signal(SIGWINCH, bp_resize);
    signal(SIGINT, clean_quit);
    bp_init();
    uint8_t ri = rand();
    uint8_t gi = rand();
    uint8_t bi = rand();
    uint8_t ro = rand() % 2;
    uint8_t go = rand() % 2;
    uint8_t bo = rand() % 2;
    if (!ro) ro = -1;
    if (!go) go = -1;
    if (!bo) bo = -1;
    uint8_t rt = 0;
    uint8_t gt = 0;
    uint8_t bt = 0;
    while (rt < ri || gt < gi || bt < bi) {
        for (int y = 0; y < bp_height; ++y) {
            for (int x = 0; x < bp_width; ++x) {
                bp_set(x, y, bp_color((uint8_t)(rt * ((double)x / ((double)bp_width - 1))), (uint8_t)(gt - (gt * ((double)x / ((double)bp_width - 1)))), (uint8_t)(bt * ((double)y / ((double)bp_height - 1)))));
            }
        }
        if (rt < ri) ++rt;
        if (gt < gi) ++gt;
        if (bt < bi) ++bt;
        bp_render();
        wait_us(25000);
    }
    while (1) {
        for (int y = 0; y < bp_height; ++y) {
            for (int x = 0; x < bp_width; ++x) {
                bp_set(x, y, bp_color((uint8_t)(ri * ((double)x / ((double)bp_width - 1))), (uint8_t)(gi - (gi * ((double)x / ((double)bp_width - 1)))), (uint8_t)(bi * ((double)y / ((double)bp_height - 1)))));
            }
        }
        if (ri == 255) ro = -1;
        else if (ri == 0) ro = 1;
        if (gi == 255) go = -1;
        else if (gi == 0) go = 1;
        if (bi == 255) bo = -1;
        else if (bi == 0) bo = 1;
        ri += ro;
        gi += go;
        bi += bo;
        bp_render();
        wait_us(25000);
    }
    bp_quit();
    return 0;
}

