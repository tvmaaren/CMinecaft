#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

#include "minecraft.h"
#include "gui.h"

#define framerate 	30.0



ALLEGRO_DISPLAY* display;
int screen_width = 640;
int screen_height = 480;
const char fontPath[] = "DejaVuSans.ttf";


int main()
{
	if(!al_init())
	{
		printf("couldn't initialize allegro\n");
		return 1;
	}
	if(!al_init_primitives_addon()){
		printf("couldn't initialize primitives addon\n");
		return 1;
	}
	if(!al_init_ttf_addon()){
		printf("couldn't initialize ttf addon\n");
		return 1;
	}

	if(!al_install_keyboard())
	{
		printf("couldn't initialize keyboard\n");
		return 1;
	}
	if(!al_install_mouse())
	{
		printf("couldn't initialize mouse\n");
		return 1;
	}

	ALLEGRO_TIMER* timer = al_create_timer(1.0 / framerate);
	if(!timer)
	{
		printf("couldn't initialize timer\n");
		return 1;
	}

	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	if(!queue)
	{
		printf("couldn't initialize queue\n");
		return 1;
	}

	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);
	display = al_create_display(screen_width, screen_height);
	if(!display)
	{
		printf("couldn't initialize display\n");
		return 1;
	}

	ALLEGRO_FONT* font = al_create_builtin_font();
	if(!font)
	{
		printf("couldn't initialize font\n");
		return 1;
	}

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	bool done = false;
	bool redraw = true;
	ALLEGRO_EVENT event;

	Button startButton;
	initButton(&startButton,"Singleplayer", 0.2,0.4,0.8,0.6,minecraft);
	Gui gui;
	Button* widgets[] = {&startButton};
	WidgetType types[] = {BUTTON};
	initGui(&gui, (void**)widgets,types,1);

	al_start_timer(timer);
	int i=0;
	while(true)
	{
		al_wait_for_event(queue, &event);
		handleGui(&gui,&event);
		al_acknowledge_resize(display);

		switch(event.type)
		{
			case ALLEGRO_EVENT_TIMER:
				// game logic goes here.
				redraw = true;
				break;

			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				screen_width=event.display.width;
				screen_height=event.display.height;
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				exit(0);
		}

		if(done)
			minecraft();

		if(redraw && al_is_event_queue_empty(queue))
		{
			//printf("%d,%d\n",i,screen_height);
			i++;
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, "Hello world!");
			drawGui(&gui);
			al_flip_display();

			redraw = false;
		}
	}

	al_destroy_font(font);
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);

	return 0;
}

// vim: cc=100
