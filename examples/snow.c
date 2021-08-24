#include <blockpix.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#ifndef _WIN32
    #include <termios.h>
    struct termios term, restore;
#endif

#ifndef SIGQUIT
#define SIGQUIT SIGTERM
#endif

int ws;
float *x = NULL;
float *y = NULL;
float *f = NULL;
float *w = NULL;
int ct;

struct timeval time1;
uint64_t tval;

uint64_t usTime()
{
    gettimeofday(&time1, NULL);
    return time1.tv_sec * 1000000 + time1.tv_usec;
}

#define frand(r) (((float)rand() / (float)(RAND_MAX)) * r)

void wait_us(uint64_t d)
{
    struct timespec dts;
    dts.tv_sec = d / 1000000;
    dts.tv_nsec = (d % 1000000) * 1000;
    nanosleep(&dts, NULL);
}

void init_snow()
{
    ct = (double)((bp_width * bp_height) / 250);
    x = (float *)realloc(x, ct * sizeof(float));
    y = (float *)realloc(y, ct * sizeof(float));
    f = (float *)realloc(f, ct * sizeof(float));
    w = (float *)realloc(w, ct * sizeof(float));
    for (int i = 0; i < ct; i++)
    {
        x[i] = (rand() % bp_width);
        y[i] = frand(bp_height);
        f[i] = frand(0.2) + 0.20;
        w[i] = (ws / 500) * (frand(1) - 0.5);
    }
}

bool resize_needed = false;

void sigwinch_hndl(int sig)
{
    (void)sig;
    resize_needed = true;
}

void exit_hndl(int sig)
{
    (void)sig;
    bp_quit();
    #ifndef _WIN32
    tcsetattr(0, TCSANOW, &restore);
    #endif
    exit(0);
}

int main(void)
{
    #ifndef _WIN32
    srand(usTime());
    tcgetattr(0, &term);
    tcgetattr(0, &restore);
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(0, TCSANOW, &term);
    #endif
    float ws = frand(150) - 75;
    bp_init();
    #ifndef _WIN32
    signal(SIGWINCH, sigwinch_hndl);
    #endif
    signal(SIGINT, exit_hndl);
    signal(SIGQUIT, exit_hndl);
    signal(SIGTERM, exit_hndl);
    brk:
    init_snow();
    while (1)
    {
        int cw = (rand() % 16) + 5;
        for (int l = 0; l <= cw; l++)
        {
            for (int i = 0; i < ct; i++)
            {
                bp_immediate_set((int)x[i], (int)y[i], 0);
                y[i] += f[i];
                x[i] += w[i];
                uint8_t tmp = (rand() % 50) + 206;
                bp_immediate_set((int)x[i], (int)y[i], bp_color(tmp, tmp, tmp));
                if ((int)y[i] > bp_height - (rand() % 3) - 1)
                {
                    x[i] = (rand() % bp_width);
                    y[i] = frand(1);
                    f[i] = frand(0.2) + 0.20;
                    w[i] = ((ws / 500) * (frand(0.25) - 0.125)) + (ws / 500);
                }
            }
            //bp_smart_render();
            double tmpr = frand(bp_width * 0.75);
            if (tmpr >= (double)(bp_width / 2))
                bp_immediate_set((rand() % bp_width), bp_height - 2, 0);
            if (resize_needed)
            {
                bp_resize();
                bp_clear();
                bp_smart_render();
                resize_needed = false;
                goto brk;
            }
            wait_us(35000);
        }
        ws = frand(150) - 75;
    }
    #ifndef _WIN32
    tcsetattr(0, TCSANOW, &restore);
    #endif
}

