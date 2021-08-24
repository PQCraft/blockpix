//▀
#include "blockpix.h"

uint32_t BLOCKPIX_LINKED_BUILD = BLOCKPIX_INCLUDE_BUILD;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wchar.h>
#ifndef _WIN32
    #include <sys/ioctl.h>
    #include <termios.h>
#else
    #include <windows.h>
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 4
    #endif
    #ifndef _O_U16TEXT
    #define _O_U16TEXT 0x00020000
    #endif
#endif

void bp_update_size();

uint32_t* _bp_data = NULL;
uint16_t _bp_dw = 0;
uint16_t _bp_dh = 0;

uint16_t bp_width = 0;
uint16_t bp_height = 0;

#ifndef _WIN32
sigset_t _bp_intmask, _bp_oldmask;
struct termios _bp_term, _bp_restore;
#endif

bool sigcheck = true;

bool bp_init() {
    if (_bp_data) return false;
    bp_update_size();
    _bp_dw = bp_width;
    _bp_dh = bp_height;
    uint32_t dsize = (_bp_dw * _bp_dh) * sizeof(uint32_t);
    if (!(_bp_data = (uint32_t*)malloc(dsize))) return false;
    memset(_bp_data, 0, dsize);
    #ifndef _WIN32
    if (sigemptyset(&_bp_intmask) == -1 || sigaddset(&_bp_intmask, SIGINT) == -1 || sigaddset(&_bp_intmask, SIGWINCH) == -1) return false;
    #else
    DWORD dwMode = 0;
    if (!GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode)) return false;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode)) return false;
    _setmode(_fileno(stdout), _O_U16TEXT);
    #endif
    for (uint16_t i = 1; i < _bp_dh / 2; ++i) {putchar('\n');}
    bp_smart_render();
    fflush(stdout);
    return true;
}

void bp_quit() {
    #ifndef _WIN32
    putchar('\n');
    #else
    putwchar('\n');
    #endif
    fflush(stdout);
    bp_silent_quit();
}

void bp_silent_quit() {
    free(_bp_data);
    _bp_data = NULL;
}

void bp_resize() {
    #ifndef _WIN32
    pthread_sigmask(SIG_SETMASK, &_bp_intmask, &_bp_oldmask);
    #endif
    bp_update_size();
    if (bp_width == _bp_dw && bp_height == _bp_dh) return;
    uint32_t dsize = (bp_width * bp_height) * sizeof(uint32_t);
    uint32_t* _bp_data_new = (uint32_t*)malloc(dsize);
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
    #ifndef _WIN32
    pthread_sigmask(SIG_SETMASK, &_bp_oldmask, NULL);
    #endif
}

void bp_update_size() {
    #ifndef _WIN32
    struct winsize max;
    ioctl(0, TIOCGWINSZ , &max);
    bp_width = max.ws_col;
    bp_height = max.ws_row * 2;
    #else
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int tmpret;
    tmpret = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    (void)tmpret;
    bp_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    bp_height = 2 * (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
    #endif
}

uint32_t bp_color(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

void bp_set(uint16_t x, uint16_t y, uint32_t c) {
    if (x < bp_width && y < bp_height) _bp_data[x + y * _bp_dw] = c;
}

void bp_immediate_set(uint16_t x, uint16_t y, uint32_t c) {
    #ifndef _WIN32
    if (sigcheck) pthread_sigmask(SIG_SETMASK, &_bp_intmask, &_bp_oldmask);
    #endif
    if (x < bp_width && y < bp_height) {
        uint32_t j = x + y * _bp_dw;
        if (_bp_data[j] == c) goto bp_immediate_set_skip;
        _bp_data[j] = c;
        y /= 2;
        uint32_t i = x + (y * 2) * _bp_dw;
        j = i + _bp_dw;
        printf("\e[s\e[%u;%uH\e[38;2;%03u;%03u;%03um\e[48;2;%03u;%03u;%03um▀\e[u", y + 1, x + 1,\
        (uint8_t)(_bp_data[i] >> 16), (uint8_t)(_bp_data[i] >> 8), (uint8_t)_bp_data[i],\
        (uint8_t)(_bp_data[j] >> 16), (uint8_t)(_bp_data[j] >> 8), (uint8_t)_bp_data[j]);
    }
    #ifndef _WIN32
    fputs("\e[0m", stdout);
    #else
    fputws(L"\e[0m", stdout);
    #endif
    fflush(stdout);
    bp_immediate_set_skip:;
    #ifndef _WIN32
    if (sigcheck) pthread_sigmask(SIG_SETMASK, &_bp_oldmask, NULL);
    #endif
}

uint32_t bp_get(uint16_t x, uint16_t y) {
    return ((x < bp_width && y < bp_height) ? _bp_data[x + y * _bp_dw] : 0);
}

void bp_render() {
    #ifndef _WIN32
    if (sigcheck) pthread_sigmask(SIG_SETMASK, &_bp_intmask, &_bp_oldmask);
    tcgetattr(0, &_bp_term);
    tcgetattr(0, &_bp_restore);
    _bp_term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(0, TCSANOW, &_bp_term);
    #endif
    uint32_t i = 0;
    uint32_t j = _bp_dw;
    uint16_t ph = bp_height / 2;
    fflush(stdout);
    for (uint16_t y = 0; y < ph;) {
        #ifndef _WIN32
        printf("\e[%u;1H", ++y);
        #else
        wprintf(L"\e[%u;1H", ++y);
        #endif
        for (uint16_t x = 0; x < bp_width; ++x) {
            #ifndef _WIN32
            printf("\e[38;2;%03u;%03u;%03um\e[48;2;%03u;%03u;%03um▀",\
            (uint8_t)(_bp_data[i] >> 16), (uint8_t)(_bp_data[i] >> 8), (uint8_t)_bp_data[i],\
            (uint8_t)(_bp_data[j] >> 16), (uint8_t)(_bp_data[j] >> 8), (uint8_t)_bp_data[j]);
            #else
            wprintf(L"\e[38;2;%03u;%03u;%03um\e[48;2;%03u;%03u;%03um▀",\
            (uint8_t)(_bp_data[i] >> 16), (uint8_t)(_bp_data[i] >> 8), (uint8_t)_bp_data[i],\
            (uint8_t)(_bp_data[j] >> 16), (uint8_t)(_bp_data[j] >> 8), (uint8_t)_bp_data[j]);
            #endif
            ++i;
            ++j;
        }
        i += _bp_dw;
        j += _bp_dw;
    }
    #ifndef _WIN32
    fputs("\e[0m", stdout);
    #else
    fputws(L"\e[0m", stdout);
    #endif
    fflush(stdout);
    #ifndef _WIN32
    tcsetattr(0, TCSANOW, &_bp_restore);
    if (sigcheck) {
        sigset_t tmpset;
        sigpending(&tmpset);
        if (sigismember(&tmpset, SIGINT) || sigismember(&tmpset, SIGWINCH)) {
            sigcheck = false;
            bp_render();
            sigcheck = true;
        }
    }
    if (sigcheck) pthread_sigmask(SIG_SETMASK, &_bp_oldmask, NULL);
    #endif
}

void bp_smart_render() {
    #ifndef _WIN32
    if (sigcheck) pthread_sigmask(SIG_SETMASK, &_bp_intmask, &_bp_oldmask);
    tcgetattr(0, &_bp_term);
    tcgetattr(0, &_bp_restore);
    _bp_term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(0, TCSANOW, &_bp_term);
    #endif
    uint32_t i = 0;
    uint32_t j = _bp_dw;
    uint16_t ph = bp_height / 2;
    uint32_t oldfg = 0;
    uint32_t oldbg = 0;
    #ifndef _WIN32
    fputs("\e[38;2;0;0;0m\e[48;2;0;0;0m", stdout);
    #else
    fputws(L"\e[38;2;0;0;0m\e[48;2;0;0;0m", stdout);
    #endif
    for (uint16_t y = 0; y < ph;) {
        #ifndef _WIN32
        printf("\e[%u;1H", ++y);
        #else
        wprintf(L"\e[%u;1H", ++y);
        #endif
        for (uint16_t x = 0; x < bp_width; ++x) {
            if (_bp_data[i] != oldfg) {
                #ifndef _WIN32
                printf("\e[38;2;%u;%u;%um",\
                (uint8_t)(_bp_data[i] >> 16), (uint8_t)(_bp_data[i] >> 8), (uint8_t)_bp_data[i]);
                #else
                wprintf(L"\e[38;2;%u;%u;%um",\
                (uint8_t)(_bp_data[i] >> 16), (uint8_t)(_bp_data[i] >> 8), (uint8_t)_bp_data[i]);
                #endif
                oldfg = _bp_data[i];
            }
            if (_bp_data[j] != oldbg) {
                #ifndef _WIN32
                printf("\e[48;2;%u;%u;%um",\
                (uint8_t)(_bp_data[j] >> 16), (uint8_t)(_bp_data[j] >> 8), (uint8_t)_bp_data[j]);
                #else
                wprintf(L"\e[48;2;%u;%u;%um",\
                (uint8_t)(_bp_data[j] >> 16), (uint8_t)(_bp_data[j] >> 8), (uint8_t)_bp_data[j]);
                #endif
                oldbg = _bp_data[j];
            }
            #ifndef _WIN32
            printf("▀");
            #else
            wprintf(L"▀");
            #endif
            ++i;
            ++j;
        }
        i += _bp_dw;
        j += _bp_dw;
    }
    #ifndef _WIN32
    fputs("\e[0m", stdout);
    #else
    fputws(L"\e[0m", stdout);
    #endif
    fflush(stdout);
    #ifndef _WIN32
    tcsetattr(0, TCSANOW, &_bp_restore);
    if (sigcheck) {
        sigset_t tmpset;
        sigpending(&tmpset);
        if (sigismember(&tmpset, SIGINT) || sigismember(&tmpset, SIGWINCH)) {
            sigcheck = false;
            bp_smart_render();
            sigcheck = true;
        }
    }
    if (sigcheck) pthread_sigmask(SIG_SETMASK, &_bp_oldmask, NULL);
    #endif
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

