#include <stdio.h>
#include <stdlib.h>
#include <blockpix.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

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
    #ifndef _WIN32
    signal(SIGWINCH, bp_resize);
    #endif
    signal(SIGINT, clean_quit);
    bp_init();
    double ri = (rand() % 256);
    double gi = (rand() % 256);
    double bi = (rand() % 256);
    double ro = (double)((rand() % 500) / 1000.0) + 0.5;
    double go = (double)((rand() % 500) / 1000.0) + 0.5;
    double bo = (double)((rand() % 500) / 1000.0) + 0.5;
    if ((rand() % 2)) ro *= -1;
    if ((rand() % 2)) go *= -1;
    if ((rand() % 2)) bo *= -1;
    double rt = 0;
    double gt = 0;
    double bt = 0;
    while (rt < ri || gt < gi || bt < bi) {
        for (int y = 0; y < bp_height; ++y) {
            for (int x = 0; x < bp_width; ++x) {
                bp_set(x, y, bp_color((uint8_t)(rt * ((double)x / ((double)bp_width - 1))), (uint8_t)(gt - (gt * ((double)x / ((double)bp_width - 1)))), (uint8_t)(bt * ((double)y / ((double)bp_height - 1)))));
            }
        }
        if (rt < ri) rt += fabs(ro);
        if (gt < gi) gt += fabs(go);
        if (bt < bi) bt += fabs(bo);
        bp_smart_render();
        wait_us(25000);
    }
    while (1) {
        for (int y = 0; y < bp_height; ++y) {
            for (int x = 0; x < bp_width; ++x) {
                bp_set(x, y, bp_color((uint8_t)(ri * ((double)x / ((double)bp_width - 1))), (uint8_t)(gi - (gi * ((double)x / ((double)bp_width - 1)))), (uint8_t)(bi * ((double)y / ((double)bp_height - 1)))));
            }
        }
        if (ri >= 255) ro *= -1;
        else if (ri <= 0) ro *= -1;
        if (gi >= 255) go *= -1;
        else if (gi <= 0) go *= -1;
        if (bi >= 255) bo *= -1;
        else if (bi <= 0) bo *= -1;
        ri += ro;
        gi += go;
        bi += bo;
        if (ri >= 255) ri = 255;
        else if (ri <= 0) ri = 0;
        if (gi >= 255) gi = 255;
        else if (gi <= 0) gi = 0;
        if (bi >= 255) bi = 255;
        else if (bi <= 0) bi = 0;
        bp_smart_render();
        wait_us(25000);
    }
    bp_quit();
    return 0;
}

