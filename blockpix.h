#ifndef BLOCKPIX
#define BLOCKPIX 21082003
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
uint32_t bp_get(uint16_t, uint16_t);
void bp_resize();
void bp_render();
void bp_smart_render();
void bp_clear();
void bp_fill(uint32_t);

#define bp_safe_set(x, y, c) if (x >= 0 && y >= 0 && x < bp_width && y < bp_height) {bp_set(x, y, c);}
#define bp_safe_get(x, y) ((x >= 0) ? ((y >= 0) ? ((x < bp_width) ? ((y < bp_height) ? bp_set(x, y, c) : 0) : 0) : 0) : 0)

#endif

