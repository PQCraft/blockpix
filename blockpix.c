//▄
#include "blockpix.h"

uint32_t BLOCKPIX_LINKED_BUILD = BLOCKPIX_INCLUDE_BUILD;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <termios.h>

void bp_update_size();

uint32_t* _bp_data = NULL;
uint16_t _bp_dw = 0;
uint16_t _bp_dh = 0;

uint16_t bp_width = 0;
uint16_t bp_height = 0;

sigset_t intmask;

struct termios term, restore;

bool bp_init() {
    if (_bp_data) return false;
    bp_update_size();
    _bp_dw = bp_width;
    _bp_dh = bp_height;
    uint32_t dsize = (_bp_dw * _bp_dh) * sizeof(uint32_t);
    if (!(_bp_data = malloc(dsize))) return false;
    memset(_bp_data, 0, dsize);
    if (sigemptyset(&intmask) == -1 || sigaddset(&intmask, SIGINT) == -1 || sigaddset(&intmask, SIGWINCH) == -1) return false;
    for (uint16_t i = 1; i < _bp_dh / 2; ++i) {putchar('\n');}
    bp_smart_render();
    return true;
}

void bp_quit() {
    putchar('\n');
    fflush(stdout);
    free(_bp_data);
    _bp_data = NULL;
}

void bp_silent_quit() {
    free(_bp_data);
    _bp_data = NULL;
}

void bp_resize() {
    sigprocmask(SIG_BLOCK, &intmask, NULL);
    bp_update_size();
    if (bp_width == _bp_dw && bp_height == _bp_dh) return;
    uint32_t dsize = (bp_width * bp_height) * sizeof(uint32_t);
    uint32_t* _bp_data_new = malloc(dsize);
    memset(_bp_data_new, 0, dsize);
    for (uint16_t y = 0; y < _bp_dh && y < bp_height; ++y) {
        uint32_t i = y * bp_width;
        uint32_t j = y * _bp_dw;
        for (uint16_t x = 0; x < _bp_dw && x < bp_width; ++x) {
            _bp_data_new[i] = _bp_data[j];
            ++i;
            ++j;
        }
    }
    free(_bp_data);
    _bp_data = _bp_data_new;
    _bp_dw = bp_width;
    _bp_dh = bp_height;
    sigprocmask(SIG_UNBLOCK, &intmask, NULL);
}

void bp_update_size() {
    struct winsize max;
    ioctl(0, TIOCGWINSZ , &max);
    bp_width = max.ws_col;
    bp_height = max.ws_row * 2;
}

uint32_t bp_color(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

void bp_set(uint16_t x, uint16_t y, uint32_t c) {
    _bp_data[x + y * _bp_dw] = c;
}

void bp_render() {
    sigprocmask(SIG_BLOCK, &intmask, NULL);
    tcgetattr(0, &term);
    tcgetattr(0, &restore);
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(0, TCSANOW, &term);
    uint32_t i = 0;
    uint32_t j = _bp_dw;
    uint16_t ph = bp_height / 2;
    for (uint16_t y = 0; y < ph;) {
        printf("\e[%u;1H", ++y);
        for (uint16_t x = 0; x < bp_width; ++x) {
            printf("\e[38;2;%03u;%03u;%03um\e[48;2;%03u;%03u;%03um▄",\
            (uint8_t)(_bp_data[j] >> 16), (uint8_t)(_bp_data[j] >> 8), (uint8_t)_bp_data[j],\
            (uint8_t)(_bp_data[i] >> 16), (uint8_t)(_bp_data[i] >> 8), (uint8_t)_bp_data[i]);
            ++i;
            ++j;
        }
        i += _bp_dw;
        j += _bp_dw;
    }
    fputs("\e[0m", stdout);
    fflush(stdout);
    tcsetattr(0, TCSANOW, &restore);
    sigprocmask(SIG_UNBLOCK, &intmask, NULL);
}

void bp_smart_render() {
    sigprocmask(SIG_BLOCK, &intmask, NULL);
    tcgetattr(0, &term);
    tcgetattr(0, &restore);
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(0, TCSANOW, &term);
    uint32_t i = 0;
    uint32_t j = _bp_dw;
    uint16_t ph = bp_height / 2;
    uint32_t oldfg = 0;
    uint32_t oldbg = 0;
    fputs("\e[38;2;0;0;0m\e[48;2;0;0;0m", stdout);
    for (uint16_t y = 0; y < ph;) {
        printf("\e[%u;1H", ++y);
        for (uint16_t x = 0; x < bp_width; ++x) {
            if (_bp_data[j] != oldfg) {
                printf("\e[38;2;%u;%u;%um",\
                (uint8_t)(_bp_data[j] >> 16), (uint8_t)(_bp_data[j] >> 8), (uint8_t)_bp_data[j]);
                oldfg = _bp_data[j];
            }
            if (_bp_data[i] != oldbg) {
                printf("\e[48;2;%u;%u;%um",\
                (uint8_t)(_bp_data[i] >> 16), (uint8_t)(_bp_data[i] >> 8), (uint8_t)_bp_data[i]);
                oldbg = _bp_data[i];
            }
            printf("▄");
            ++i;
            ++j;
        }
        i += _bp_dw;
        j += _bp_dw;
    }
    fputs("\e[0m", stdout);
    fflush(stdout);
    tcsetattr(0, TCSANOW, &restore);
    sigprocmask(SIG_UNBLOCK, &intmask, NULL);
}

void bp_clear() {
    memset(_bp_data, 0, (_bp_dw * _bp_dh) * sizeof(uint32_t));
}

void bp_fill(uint32_t c) {
    uint32_t dsize = (_bp_dw * _bp_dh) * sizeof(uint32_t);
    for (uint32_t dpos = 0; dpos < dsize; ++dpos) {
        _bp_data[dpos] = c;
    }
}

