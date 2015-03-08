
#include <xrl/xrl.h>

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "image.h"

extern const char tilemap_frag_asm[];

static uint8_t * indices;
static uint8_t * colors;
static uint8_t * bgcolors;

static int res_w = 0;
static int res_h = 0;

static GLuint tileset_texture;
static GLuint indices_texture;
static GLuint color_texture;
static GLuint bgcolor_texture;
static int tile_w = 0;
static int tile_h = 0;

static int cursor_x = 0;
static int cursor_y = 0;

static SDL_Window * window = 0;

static GLuint tilemap_prog;

static void texture_data(GLuint id, const uint8_t * image_data, size_t w, size_t h)
{
	glBindTexture(GL_TEXTURE_2D, id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
}
static GLuint new_texture()
{
	GLuint id = 0;

	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return id;
}

bool xrl_init(size_t w, size_t h, const char * fontpath)
{
	SDL_Init(SDL_INIT_VIDEO);

	res_w = w;
	res_h = h;

	size_t fontimg_w;
	size_t fontimg_h;
	uint8_t * fontimg_data;

	image_load_png(fontpath, &fontimg_data, &fontimg_w, &fontimg_h);

	if(fontimg_w % 16 != 0 || fontimg_h % 16 != 0)
		printf("Bad font size %ux%u\n", fontimg_w, fontimg_h);

	tile_w = fontimg_w / 16;
	tile_h = fontimg_h / 16;

	int width = tile_w*res_w;
	int height = tile_h*res_h;

	window = SDL_CreateWindow("quantumchamp", 100, 100, width, height, SDL_WINDOW_OPENGL);

	SDL_GL_CreateContext(window);

	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		printf("glewInit() failed.\n");
		return false;
	}

	if(!GLEW_ARB_fragment_program)
	{
		printf("GLEW_ARB_fragment_program is unavailable.\n");
		return false;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);


	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	glGenProgramsARB(1, &tilemap_prog);

	if(!tilemap_prog)
	{
		printf("Failed to generate fragment program\n");
		return false;
	}

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, tilemap_prog);

	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(tilemap_frag_asm), tilemap_frag_asm);

	GLuint error = glGetError();

	if(error == GL_INVALID_OPERATION)
	{
		printf("GL_INVALID_OPERATION!\n");

		printf("glGetString(GL_PROGRAM_ERROR_STRING_ARB): %s\n", glGetString(GL_PROGRAM_ERROR_STRING_ARB));

		GLint texture_units;
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &texture_units);
		printf("GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB: %d\n", texture_units);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &texture_units);
		printf("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB: %d\n", texture_units);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &texture_units);
		printf("GL_MAX_TEXTURE_IMAGE_UNITS_ARB: %d\n", texture_units);
		glGetIntegerv(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB, &texture_units);
		printf("GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB: %d\n", texture_units);

		return false;
	}

	// Window size
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, width, height, 00.0, 00.0);
	// Font output size and inverse font output size
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, 10.0, 10.0, 1/10.0, 1/10.0);


	tileset_texture = new_texture();
	texture_data(tileset_texture, fontimg_data, fontimg_w, fontimg_h);

	indices_texture = new_texture();
	color_texture = new_texture();
	bgcolor_texture = new_texture();

	//printf("%d, %d, %d, %d\n", tileset_texture, indices_texture, color_texture, bgcolor_texture);

	glViewport(0, 0, width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Allocate the character buffers
	indices = malloc(3*res_w*res_h);
	colors = malloc(3*res_w*res_h);
	bgcolors = malloc(3*res_w*res_h);

	for(int k = 0 ; k < res_w*res_h ; k ++)
	{
		//Everything is black blank initially
		indices[k*3 + 0] = 0;
		indices[k*3 + 1] = 0;
		indices[k*3 + 2] = 0; // Old versions of opengl don't support GL_RG, so this needs to be in RGB
		colors[k*3 + 0] = 0;
		colors[k*3 + 1] = 0;
		colors[k*3 + 2] = 0;
		bgcolors[k*3 + 0] = 0;
		bgcolors[k*3 + 1] = 0;
		bgcolors[k*3 + 2] = 0;
	}

	return true;
}
void xrl_deinit()
{
	free(indices);
	free(colors);
	free(bgcolors);
	indices = 0;
	colors = 0;
	bgcolors = 0;

	glDeleteTextures(1, &tileset_texture);
	glDeleteTextures(1, &indices_texture);
	glDeleteTextures(1, &color_texture);

	glDeleteProgram(tilemap_prog);

	SDL_DestroyWindow(window);

	SDL_Quit();
}

typedef struct color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color_t;

static color_t fgcolor = { 255, 255, 255 };
static color_t bgcolor = {   0,   0,   0 };

void xrl_moveto(size_t x, size_t y)
{
	cursor_x = x;
	cursor_y = y;
}
void xrl_fgcolor(uint8_t r, uint8_t g, uint8_t b)
{
	fgcolor.r = r;
	fgcolor.g = g;
	fgcolor.b = b;
}
void xrl_bgcolor(uint8_t r, uint8_t g, uint8_t b)
{
	bgcolor.r = r;
	bgcolor.g = g;
	bgcolor.b = b;
}
static void xrl_setc(uint8_t c, size_t x, size_t y)
{
	//printf("draw_char('%c' (%3d), %u, %u)\n", c, (int)c, x, y);

	if(x < res_w && y < res_h)
	{
		uint8_t x_idx = c % 16;
		uint8_t y_idx = c / 16;

		//printf("color: %X,%X,%X\n", color.r, color.g, color.b);

		indices[3*(x + y*res_w) + 0] = x_idx * 16;
		indices[3*(x + y*res_w) + 1] = y_idx * 16;
		indices[3*(x + y*res_w) + 2] = 0;
		colors[3*(x + y*res_w) + 0] = fgcolor.r;
		colors[3*(x + y*res_w) + 1] = fgcolor.g;
		colors[3*(x + y*res_w) + 2] = fgcolor.b;
		bgcolors[3*(x + y*res_w) + 0] = bgcolor.r;
		bgcolors[3*(x + y*res_w) + 1] = bgcolor.g;
		bgcolors[3*(x + y*res_w) + 2] = bgcolor.b;
	}
}
void xrl_putc(uint8_t c)
{
	xrl_setc(c, cursor_x, cursor_y);

	cursor_x += 1;
	if(cursor_x > res_w)
		cursor_x = res_w;
}
void xrl_putstr(const char * str)
{
	//printf("parsing \"%s\"\n", str);

	size_t x = cursor_x;
	size_t y = cursor_y;

	const char * c = str;

	while(*c != '\0')
	{
		if(*c == '%')
		{
			c ++; if(*c == '\0') break;

			if(*c == '%')
			{
				xrl_setc('%', x, y);
				x++;

				c ++; if(*c == '\0') break;

				continue;
			}
			else if(*c == 'n')
			{
				y++;
				x = cursor_x;

				c ++; if(*c == '\0') break;

				continue;
			}
			else
			{
				uint32_t fgc;
				if(sscanf(c, "%6X", &fgc) == 1)
				{
					fgcolor.r = (fgc >> 16) & 0xFF;
					fgcolor.g = (fgc >>  8) & 0xFF;
					fgcolor.b = (fgc      ) & 0xFF;

					c ++; if(*c == '\0') break;
					c ++; if(*c == '\0') break;
					c ++; if(*c == '\0') break;
					c ++; if(*c == '\0') break;
					c ++; if(*c == '\0') break;
					c ++; if(*c == '\0') break;

					if(*c == '/')
					{
						c ++; if(*c == '\0') break;

						uint32_t bgc;
						if(sscanf(c, "%6X", &bgc) == 1)
						{
							bgcolor.r = (bgc >> 16) & 0xFF;
							bgcolor.g = (bgc >>  8) & 0xFF;
							bgcolor.b = (bgc      ) & 0xFF;
						}
						else break;

						c ++; if(*c == '\0') break;
						c ++; if(*c == '\0') break;
						c ++; if(*c == '\0') break;
						c ++; if(*c == '\0') break;
						c ++; if(*c == '\0') break;
						c ++; if(*c == '\0') break;
					}
				}
				else break;
			}
		}
		else
		{
			// write char
			xrl_setc(*c, x, y);
			x++;

			c ++; if(*c == '\0') break;
		}
	}

	cursor_x = x;
	cursor_y = y;
}

void xrl_flush()
{
	//  ----------------- Assign some uniforms in the fragment shader -----------------
	glUseProgram(tilemap_prog);

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, indices_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res_w, res_h, 0, GL_RGB, GL_UNSIGNED_BYTE, indices);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res_w, res_h, 0, GL_RGB, GL_UNSIGNED_BYTE, colors);

	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bgcolor_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res_w, res_h, 0, GL_RGB, GL_UNSIGNED_BYTE, bgcolors);

	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tileset_texture);

	//cout << _tileMap->tilesTexture << ", " << _tileMap->indexTexture << ", " << _tileMap->colorTexture << endl;

	glActiveTextureARB(GL_TEXTURE0);

	glUniform2f(1, res_w, res_h);
	glUniform2f(0, 16, 16);

	glBegin(GL_QUADS);

		glTexCoord2f(0, 0); glVertex2f(-1,  1);
		glTexCoord2f(1, 0); glVertex2f( 1,  1);
		glTexCoord2f(1, 1); glVertex2f( 1, -1);
		glTexCoord2f(0, 1); glVertex2f(-1, -1);

	glEnd();


	glUseProgram(0);

	SDL_GL_SwapWindow(window);
}
void xrl_clear()
{
	//Make sure to update the buffer
	for(int k = 0 ; k < res_w*res_h ; k ++)
	{
		indices[k*3 + 0] = 0;
		indices[k*3 + 1] = 0;
		indices[k*3 + 0] = 0;
		colors[k*3 + 0] = 0;
		colors[k*3 + 1] = 0;
		colors[k*3 + 2] = 0;
		bgcolors[k*3 + 0] = 0;
		bgcolors[k*3 + 1] = 0;
		bgcolors[k*3 + 2] = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT);
}

/*
static uint8_t get_xrl_key(SDL_KeyboardEvent sdl_key)
{
	switch(sdl_key.keysym.sym)
	{
		case SDLK_SPACE:
			return XRLK_SPACE;
		case SDLK_EXCLAIM:
			return XRLK_EXCLAMATION;
		case SDLK_QUOTEDBL:
			return XRLK_DOUBLEQUOTE;
		case SDLK_HASH:
			return XRLK_HASH;
		case SDLK_DOLLAR:
			return XRLK_DOLLAR;
		case SDLK_PERCENT:
			return XRLK_PERCENT;
		case SDLK_AMPERSAND:
			return XRLK_AMPERSAND;
		case SDLK_QUOTE:
			return XRLK_SINGLEQUOTE;
		case SDLK_LEFTPAREN:
			return XRLK_OPENPAREN;
		case SDLK_RIGHTPAREN:
			return XRLK_CLOSEPAREN;
		case SDLK_ASTERISK:
			return XRLK_ASTERISK;
		case SDLK_PLUS:
			return XRLK_PLUS;
		case SDLK_COMMA:
			return XRLK_COMMA;
		case SDLK_MINUS:
			return XRLK_MINUS;
		case SDLK_PERIOD:
			return XRLK_PERIOD;
		case SDLK_SLASH:
			return XRLK_SLASH;
		case SDLK_0:
			return XRLK_0;
		case SDLK_1:
			return XRLK_1;
		case SDLK_2:
			return XRLK_2;
		case SDLK_3:
			return XRLK_3;
		case SDLK_4:
			return XRLK_4;
		case SDLK_5:
			return XRLK_5;
		case SDLK_6:
			return XRLK_6;
		case SDLK_7:
			return XRLK_7;
		case SDLK_8:
			return XRLK_8;
		case SDLK_9:
			return XRLK_9;
		case SDLK_COLON:
			return XRLK_COLON;
		case SDLK_SEMICOLON:
			return XRLK_SEMICOLON;
		case SDLK_LESS:
			return XRLK_LESSTHAN;
		case SDLK_EQUALS:
			return XRLK_EQUAL;
		case SDLK_GREATER:
			return XRLK_GREATERTHAN;
		case SDLK_QUESTION:
			return XRLK_QUESTION;
		case SDLK_AT:
			return XRLK_AT;
		case SDLK_a:
			return XRLK_a;
		case SDLK_b:
			return XRLK_b;
		case SDLK_c:
			return XRLK_c;
		case SDLK_d:
			return XRLK_d;
		case SDLK_e:
			return XRLK_e;
		case SDLK_f:
			return XRLK_f;
		case SDLK_g:
			return XRLK_g;
		case SDLK_h:
			return XRLK_h;
		case SDLK_i:
			return XRLK_i;
		case SDLK_j:
			return XRLK_j;
		case SDLK_k:
			return XRLK_k;
		case SDLK_l:
			return XRLK_l;
		case SDLK_m:
			return XRLK_m;
		case SDLK_n:
			return XRLK_n;
		case SDLK_o:
			return XRLK_o;
		case SDLK_p:
			return XRLK_p;
		case SDLK_q:
			return XRLK_q;
			return XRLK_r;
			return XRLK_s;
			return XRLK_t;
			return XRLK_u;
			return XRLK_v;
			return XRLK_w;
			return XRLK_x;
			return XRLK_y;
			return XRLK_z;
			return XRLK_LEFTBRACKET;
			return XRLK_BACKSLASH;
			return XRLK_RIGHTBRACKET;
			return XRLK_CARET;
			return XRLK_UNDERSCORE;
			return XRLK_BACKTICK;
			return XRLK_VERTICALBAR;
			return XRLK_TILDE;
	}

	return XRLK_NONE;
}
*/

uint32_t xrl_ticks()
{
	return SDL_GetTicks();
}

bool xrl_pollevent(xrl_event_t * event)
{
	SDL_Event e;

	if(SDL_PollEvent(&e))
	{
		if(e.type == SDL_KEYDOWN)
		{
			event->type = XRL_KEYDOWN;
			event->key = e.key.keysym.scancode;
			return true;
		}
		else if(e.type == SDL_KEYUP)
		{
			event->type = XRL_KEYUP;
			event->key = e.key.keysym.scancode;
			return true;
		}
		else if(e.type == SDL_MOUSEMOTION)
		{
			event->type = XRL_MOUSEMOVED;
			event->mouse.x = e.motion.x;
			event->mouse.y = e.motion.y;
			event->mouse.dx = e.motion.xrel;
			event->mouse.dy = e.motion.yrel;
			event->mouse.button = XRL_MOUSE_NONE;
			return true;
		}
		else if(e.type == SDL_MOUSEBUTTONDOWN)
		{
			event->type = XRL_MOUSEDOWN;
			event->mouse.x = e.button.x;
			event->mouse.y = e.button.y;
			event->mouse.dx = 0;
			event->mouse.dy = 0;

			if(e.button.button == SDL_BUTTON_LEFT)
			{
				event->mouse.button = XRL_MOUSE_LEFT;
			}
			else if(e.button.button == SDL_BUTTON_RIGHT)
			{
				event->mouse.button = XRL_MOUSE_RIGHT;
			}
			else
			{
				event->mouse.button = XRL_MOUSE_NONE;
			}
			return true;
		}
		else if(e.type == SDL_MOUSEBUTTONUP)
		{
			event->type = XRL_MOUSEUP;
			event->mouse.x = e.button.x;
			event->mouse.y = e.button.y;
			event->mouse.dx = 0;
			event->mouse.dy = 0;

			if(e.button.button == SDL_BUTTON_LEFT)
			{
				event->mouse.button = XRL_MOUSE_LEFT;
			}
			else if(e.button.button == SDL_BUTTON_RIGHT)
			{
				event->mouse.button = XRL_MOUSE_RIGHT;
			}
			else
			{
				event->mouse.button = XRL_MOUSE_NONE;
			}
			return true;
		}
		else if(e.type == SDL_QUIT)
		{
			event->type = XRL_QUIT;
			return true;
		}
	}

	return false;
}
