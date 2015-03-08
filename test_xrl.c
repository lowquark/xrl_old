
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <xrl/xrl.h>

bool running = true;

size_t width = 80, height = 50;

int main(int argc, char ** argv)
{
	if(!xrl_init(width, height, "Potash_10x10.png"))
	{
		printf("Failed to initialize graphics\n");
	}

	size_t drawcount = 0;
	uint32_t first_tick = xrl_ticks();

	int x = 0, y = 0;

	while(running)
	{
		xrl_event_t event;
		while(xrl_pollevent(&event))
		{
			if(event.type == XRL_QUIT)
			{
				running = false;
			}
			else if(event.type == XRL_KEYDOWN)
			{
				if(event.key == XRL_KEY_0)
					running = false;
			}
			else if(event.type == XRL_MOUSEMOVED)
			{
				x = event.mouse.x / 10;
				y = event.mouse.y / 10;
			}
		}

		//printf("GOT HERE\n");

		xrl_clear();

		size_t rx = 40, ry = 25;
		for(int i = 0;i < 100;i ++)
		{
			xrl_moveto(rx, ry);
			xrl_fgcolor(255, 255, 0);
			xrl_bgcolor(rand() % 256, rand() % 256, rand() % 256);
			xrl_putc('A');

			if(rand() % 2 == 0)
			{
				if(x > rx)
					rx += (rand() % 2);
				if(x < rx)
					rx -= (rand() % 2);
				if(y > ry)
					ry += (rand() % 2);
				if(y < ry)
					ry -= (rand() % 2);
			}
			else
			{
				rx += ((rand() % 3) - 1);
				ry += ((rand() % 3) - 1);
			}
		}

		xrl_moveto(x-3, y);
		xrl_putstr("%000000/808080xrl xrl%nxrl xrl%n%773300/FF00FF%%shit%%");
		// "%FF0000/808080" "%n"

		xrl_flush();

		drawcount ++;
	}

	uint32_t tms = xrl_ticks() - first_tick;

	printf("%.2f fps\n", (float)drawcount/tms*1000.0f);

	xrl_deinit();
}

