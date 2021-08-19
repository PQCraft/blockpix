#ifndef BLOCKPIX
#define BLOCKPIX

#include <stdint.h>
#include <stdbool.h>

extern uint16_t bp_width;
extern uint16_t bp_height;

bool bp_init();
void bp_quit();
uint32_t bp_color(uint8_t, uint8_t, uint8_t);
void bp_set(uint16_t, uint16_t, uint32_t);
void bp_resize();
void bp_render();

#endif

