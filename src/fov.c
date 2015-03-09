
#include "ppfov.h"

#undef DEBUG

#ifdef DEBUG

#include <SDL2/SDL.h>
#include <stdio.h>

/*
static void wait_for_input()
{
	while(true)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_KEYDOWN)
			{
				return;
			}
		}
	}
}

void debug_draw(size_t x, size_t y)
{
	if(DEBUG)
	{
		fflush(stdout);

		render_state(x, y);

		wait_for_input();
	}
}
*/

/*
static void print_views()
{
	for(int i = 0;i < view_num;i ++)
	{
		printf("%d: { (%4d,%4d) => (%4d,%4d), (%4d,%4d) => (%4d,%4d), %s }\n", i,
			views[i].shallow.near.x, views[i].shallow.near.y,
			views[i].shallow.far.x, views[i].shallow.far.y,
			views[i].steep.near.x, views[i].steep.near.y,
			views[i].steep.far.x, views[i].steep.far.y,
			views[i].active ? "ACTIVE" : "INACTIVE"
		);
	}
}
*/

#define PRINTF(...) (printf(__VA_ARGS__))

#else

#define PRINTF(...) (0)

#endif

typedef struct point
{
	int x, y;
} point_t;
typedef struct line
{
	point_t near;
	point_t far;
} line_t;
typedef struct view
{
	line_t shallow, steep;
	point_t shallow_bumps[30];
	size_t shallow_bump_num;
	point_t steep_bumps[30];
	size_t steep_bump_num;
	bool active;
} view_t;

size_t view_num = 0;
view_t views[100] = {};

static void add_view(const view_t * view)
{
	if(view_num < 100)
	{
		views[view_num] = *view;
		view_num ++;
	}
}

/*
static void remove_view(const view_t * view)
{
	for(size_t i = 0;i < view_num;i ++)
	{
		if(&views[i] == view)
		{
			for(size_t d = i; d < view_num - 1; d ++)
			{
				views[d] = views[d + 1];
			}

			view_num --;

			break;
		}
	}
}
*/

typedef enum {
	OUTSIDE,
	ENCLOSED,
	INTERSECT_SHALLOW,
	INTERSECT_STEEP,
	INTERSECT_BOTH
} viewresult_t;

static inline bool view_below_shallow(const view_t * view, point_t * p)
{
	return (p->y - view->shallow.near.y) * (view->shallow.far.x - view->shallow.near.x) <= (p->x - view->shallow.near.x) * (view->shallow.far.y - view->shallow.near.y);
}
static inline bool view_above_steep(const view_t * view, point_t * p)
{
	return (p->y - view->steep.near.y) * (view->steep.far.x - view->steep.near.x) >= (p->x - view->steep.near.x) * (view->steep.far.y - view->steep.near.y);
}
static int process_view(const view_t * view, point_t d, point_t e)
{
	//     sh-->

	// st  c   d
	// |     #
	// v   e   f

	PRINTF("d: (%d,%d)\n", d.x, d.y);
	PRINTF("e: (%d,%d)\n", e.x, e.y);

	bool d_outside_sh = view_below_shallow(view, &d);
	bool e_outside_sh = view_below_shallow(view, &e);
	bool d_outside_st = view_above_steep(view, &d);
	bool e_outside_st = view_above_steep(view, &e);

	PRINTF("d_outside_sh: %s\n", d_outside_sh ? "true" : "false");
	PRINTF("e_outside_sh: %s\n", e_outside_sh ? "true" : "false");
	PRINTF("d_outside_st: %s\n", d_outside_st ? "true" : "false");
	PRINTF("e_outside_st: %s\n", e_outside_st ? "true" : "false");

	if(d_outside_st)
		return OUTSIDE;
	if(e_outside_sh)
		return OUTSIDE;
	if(e_outside_st && d_outside_sh)
		return INTERSECT_BOTH;
	if(d_outside_sh)
		return INTERSECT_SHALLOW;
	if(e_outside_st)
		return INTERSECT_STEEP;

	return ENCLOSED;
}
static void view_bump_shallow(view_t * view, int x, int y)
{
	PRINTF("Bumping shallow\n");

	point_t corner = { x, y + 1 };

	view->shallow.far = corner;

	if(view->shallow_bump_num < 30)
	{
		view->shallow_bumps[view->shallow_bump_num] = corner;
		view->shallow_bump_num ++;
	}

	PRINTF("steep bumps: %d\n", view->steep_bump_num);

	for(int b = 0; b < view->steep_bump_num; b ++)
	{
		point_t * p = &view->steep_bumps[b];
		PRINTF("[%d] => (%d, %d)\n", b, p->x, p->y);

		if(view_below_shallow(view, p))
		{
			view->shallow.near = *p;
			PRINTF("Below shallow!\n");
		}
	}
}
static void view_bump_steep(view_t * view, int x, int y)
{
	PRINTF("Bumping steep\n");

	point_t corner = { x + 1, y };

	view->steep.far = corner;

	if(view->steep_bump_num < 30)
	{
		view->steep_bumps[view->steep_bump_num] = corner;
		view->steep_bump_num ++;
	}

	PRINTF("shallow bumps: %d\n", view->shallow_bump_num);

	for(int b = 0; b < view->shallow_bump_num; b ++)
	{
		point_t * p = &view->shallow_bumps[b];
		PRINTF("[%d] => (%d, %d)\n", b, p->x, p->y);

		if(view_above_steep(view, p))
		{
			view->steep.near = *p;
			PRINTF("Above steep!\n");
		}
	}
}

static bool process_tile(size_t x, size_t y, char tile)
{
	PRINTF("\n----(%d, %d)----\n", x, y);

	view_t newview;
	bool addnew = false;

	bool visible = false;

	for(int v = 0; v < view_num; v ++)
	{
		view_t * view = &views[v];

		if(view->active)
		{
			point_t xcorner = { x + 1, y     };
			point_t ycorner = { x    , y + 1 };

			//     sh-->

			// st  c   d
			// |     #
			// v   e   f

			//return d_outside_a & (e_outside_a << 1) & (d_outside_b << 2) & (e_outside_b << 3);

			PRINTF("--view %d--\n", v);
			int val = process_view(view, xcorner, ycorner);

			if(val == INTERSECT_BOTH)
			{
				PRINTF("Intersect both\n");

				if(tile)
				{
					//Terminate node
					view->active = false;

					//PRINTF("Terminating view\n");
				}

				visible = true;
			}
			else if(val == INTERSECT_SHALLOW)
			{
				PRINTF("Shallow intersect\n");

				if(tile)
				{
					view_bump_shallow(view, x, y);
				}

				visible = true;
			}
			else if(val == INTERSECT_STEEP)
			{
				PRINTF("Steep intersect\n");

				if(tile)
				{
					view_bump_steep(view, x, y);
				}

				visible = true;
			}
			else if(val == ENCLOSED)
			{
				if(tile)
				{
					newview = *view;

					newview.steep_bump_num = 0;
					view_bump_steep(&newview, x, y);

					view->shallow_bump_num = 0;
					view_bump_shallow(view, x, y);

					addnew = true;
				}

				visible = true;

				PRINTF("Enclosed\n");
			}
			if(val == OUTSIDE)
			{
				PRINTF("Outside\n");
			}
		}
	}

	if(addnew)
	{
		//PRINTF("adding new view\n");
		add_view(&newview);
		//print_views();
	}

	return visible;
}

static void init_views()
{
	view_num = 0;

	view_t init = { .shallow = { { 0, 1 }, { 200,  0 } },
	                .steep =   { { 1, 0 }, {  0, 200 } },
	                .active = true,
	                .shallow_bump_num = 0,
	                .steep_bump_num = 0};

	add_view(&init);
}

static void process_quad(uint32_t quad, bool * litmap, const bool * solidmap, size_t width, size_t height, size_t src_x, size_t src_y)
{
	init_views();

	for(int stride = 1; stride < 11; stride ++)
	{
		for(int i = 0; i <= stride; i ++)
		{
			size_t x, y;

			switch(quad)
			{
				case 4:
					x = src_x + i;
					y = i - stride + src_y;
					break;
				case 3:
					x = src_x - i;
					y = i - stride + src_y;
					break;
				case 2:
					x = src_x - i;
					y = stride - i + src_y;
					break;
				case 1:
				default:
					x = i + src_x;
					y = stride - i + src_y;
					break;
			}

			uint8_t solid = 0;

			if(x < width && y < height)
				solid = solidmap[x + y*width];

			bool visible = process_tile(i, stride - i, solid);

			if(x < width && y < height)
				litmap[x + y*width] = visible;

			//debug_draw(x, y);

			if(view_num == 0)
				return;
		}
	}
}

__declspec(dllexport)
void xrl_compute_fov(bool * litmap, const bool * solidmap, size_t width, size_t height, size_t src_x, size_t src_y)
{
	for(int j = 0; j < width; j ++)
	{
		for(int i = 0; i < height; i ++)
		{
			litmap[i + j*width] = 0;
		}
	}

	if(src_x < width && src_y < height)
		litmap[src_x + src_y*width] = 1;

	process_quad(1, litmap, solidmap, width, height, src_x, src_y);
	process_quad(2, litmap, solidmap, width, height, src_x, src_y);
	process_quad(3, litmap, solidmap, width, height, src_x, src_y);
	process_quad(4, litmap, solidmap, width, height, src_x, src_y);
}

