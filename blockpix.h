#ifndef BLOCKPIX
#define BLOCKPIX 2021082200
#define BLOCKPIX_INCLUDE_BUILD BLOCKPIX

#include <stdint.h>
#include <stdbool.h>

extern uint32_t BLOCKPIX_LINKED_BUILD;
extern uint16_t bp_width;
extern uint16_t bp_height;

bool bp_init();
void bp_quit();
void bp_silent_quit();
uint32_t bp_color(uint8_t, uint8_t, uint8_t);
void bp_set(uint16_t, uint16_t, uint32_t);
void bp_immediate_set(uint16_t, uint16_t, uint32_t);
uint32_t bp_get(uint16_t, uint16_t);
void bp_draw_line(int, int, int, int, uint32_t);
void bp_resize();
void bp_render();
void bp_smart_render();
void bp_clear();
void bp_fill(uint32_t);

#endif

