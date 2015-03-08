#ifndef XRL_H
#define XRL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <xrl/keys.h>

typedef struct xrl_event
{
	enum {
		XRL_KEYDOWN,
		XRL_KEYUP,
		XRL_MOUSEMOVED,
		XRL_MOUSEDOWN,
		XRL_MOUSEUP,
		XRL_QUIT
	} type;

	union
	{
		uint8_t key;
		struct {
			int x;
			int y;
			int dx;
			int dy;

			enum { XRL_MOUSE_LEFT, 
			       XRL_MOUSE_RIGHT,
				   XRL_MOUSE_NONE } button;
		} mouse;
	};
} xrl_event_t;

bool xrl_init(size_t w, size_t h, const char * fontpath);
void xrl_deinit();

void xrl_moveto(size_t x, size_t y);
void xrl_fgcolor(uint8_t r, uint8_t g, uint8_t b);
void xrl_bgcolor(uint8_t r, uint8_t g, uint8_t b);

// modify cursor
void xrl_putc(uint8_t c);
void xrl_putstr(const char * str);

void xrl_flush();
void xrl_clear();

uint32_t xrl_ticks();

bool xrl_pollevent(xrl_event_t * event);

#endif
