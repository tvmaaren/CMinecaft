#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "types.h"
#include "list.h"
#include "block.h"
#include "world.h"

#define FRAMERATE 60.0
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define SPEED 5.0
#define GRAVITY -0.5

List world;




void set_perspective_transform(ALLEGRO_BITMAP* bmp,Player player)
{
	ALLEGRO_TRANSFORM p;
	float aspect_ratio = (float)al_get_bitmap_height(bmp) / al_get_bitmap_width(bmp);
	al_set_target_bitmap(bmp);
	al_identity_transform(&p);
	al_translate_transform_3d(&p, -player.pos.x*SIZE_BLOCK,
		   -player.pos.y*SIZE_BLOCK, player.pos.z*SIZE_BLOCK);
	al_rotate_transform_3d(&p,0,1,0,player.hor_angle);
	al_rotate_transform_3d(&p,1,0,0,player.vert_angle);
	al_perspective_transform(&p, -1, aspect_ratio, 1, 1, -aspect_ratio, 1000);
	al_use_projection_transform(&p);
}



bool isValidPlayerPos(Pos pos, unsigned int chunk_index){
	Pos pos_low=pos;
	pos_low.y--;
	return(is_block(&world,pos_low,chunk_index)==UINT_MAX//Checks if the lower part of the player's body is
					 //in a valid position
			&& is_block(&world,pos,chunk_index)==UINT_MAX);
}



void move(float step, Player* p,float angle,bool checkValid){
	float x = cos(p->hor_angle+angle);
	float y = sin(p->hor_angle+angle);
	Pos new_pos = p->pos;
	Pos new_pos_extra = p->pos;
	new_pos.z+=step*x;
	new_pos.x+=step*y;
	
	new_pos_extra.z+=(step+0.1)*x;
	new_pos_extra.x+=(step+0.1)*y;

	//check if the chunk index is still correct
	unsigned int new_chunk_index = getChunk(&world,new_pos, p->chunk_index);

	if(!checkValid ||  isValidPlayerPos(new_pos_extra,new_chunk_index)){
		p->pos=new_pos;
		p->chunk_index=new_chunk_index;
	}
}

unsigned int ray_block(Direction* dir,Vec v,Pos pos,unsigned int chunk_index){
	for(unsigned int i=0;i<100;i++){
		Pos prev_pos = pos;
		pos =(Pos)ADD_VEC(v,pos);
		unsigned int ret;
		if((ret=is_block(&world,pos,chunk_index))!=UINT_MAX){
			if(dir)*dir=hitDirectionBlock(prev_pos,
					INDEX_TO_POS(&world,ret));
			return(ret);
		}
	}
	return(UINT_MAX);
}

unsigned int ray_block_from_player(Direction* dir,Player p,unsigned int chunk_index){
	Vec v = {0.5*sin(p.hor_angle)*cos(p.vert_angle), 0.5*sin(-p.vert_angle),
		0.5*cos(p.hor_angle)*cos(p.vert_angle)};
	return(ray_block(dir, v,p.pos,chunk_index));
}

//Gives the distance of x to [a,b] where a<=b
float distanceInterval(float x,float a,float b){
	if(x<=a)return(a-x);
	else if(b<=x)return(x-b);
	else return(0);
}


int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display;
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *queue;
	ALLEGRO_BITMAP *display_2d;
	ALLEGRO_BITMAP *bottem;
	ALLEGRO_BITMAP *top;
	ALLEGRO_BITMAP *side;
   	ALLEGRO_FONT* font;
	
	
	Player player={0,0,{1000,25,1000},UINT_MAX,true,0};
	bool redraw = false;
	bool quit = false;
	bool fullscreen = false;
	bool background = false;
	int display_flags = ALLEGRO_RESIZABLE;

	const char walkingStr[] ="Walking";
	const char flyingStr[] = "Flying";
	bool flying = false;
	
	
	if (!al_init()) {
		printf("Could not init Allegro.\n");
		exit(1);
	}
	al_init_image_addon();
	loadBlocks();
   	al_init_font_addon();
	font = al_create_builtin_font();
	
	unsigned int standing_on = UINT_MAX;
	al_init_primitives_addon();
	al_install_keyboard();
	al_install_mouse();
	
	al_set_new_display_flags(display_flags);
	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);
	display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
	al_hide_mouse_cursor(display);
	al_grab_mouse(display);
	al_set_mouse_xy(display,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	if (!display) {
	   printf("Error creating display\n");
	   exit(1);
	}
	
	display_2d = al_create_sub_bitmap(al_get_backbuffer(display), 0, 0,
	     	   SCREEN_WIDTH, SCREEN_HEIGHT);
	
	timer = al_create_timer(1.0 / FRAMERATE);
	
	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

#define KEY_SEEN     1
#define KEY_RELEASED 2
	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));
	
	BlockType block_selection=GRASS_BLOCK;
	world= list_init(0);
	
	
	al_start_timer(timer);
	while (!quit) {
		loadChunks(&world,player);

		if(!flying){
		Pos new_pos = player.pos;
		new_pos.y+=player.vert_speed/FRAMERATE;

		Pos foot;
		foot=new_pos;
		foot.y-=1.8;
		unsigned int b= is_block(&world,foot,player.chunk_index);
		if(b==UINT_MAX){
			Pos prev_block_pos;
			if(!player.falling)
				prev_block_pos = INDEX_TO_POS(&world,standing_on);
			if(	standing_on==UINT_MAX ||
				WORLD_BLOCK_INDEX(&world,standing_on)==AIR
				|| player.falling
				//check if the player is still standing on a block
				|| distanceInterval(player.pos.x,prev_block_pos.x,
					prev_block_pos.x+1.0)>0.5 
				|| distanceInterval(player.pos.z,prev_block_pos.z,
						prev_block_pos.z+1.0)>0.5){


				player.falling=true;
				player.pos.y+=player.vert_speed;
			}
		}
		else{
			standing_on=b;
			player.falling=false;
			player.pos.y=ceilf(player.pos.y-1.8)+1.8;
			player.vert_speed=0;
		}
		if(player.falling)player.vert_speed+=GRAVITY/FRAMERATE;
		}
		ALLEGRO_EVENT event;
		
		al_wait_for_event(queue, &event);
		switch (event.type) {
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				quit = true;
				break;
			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				al_acknowledge_resize(display);
				break;
			case ALLEGRO_EVENT_MOUSE_AXES:
				//wraps the mouse around if necessary
				if(event.mouse.x<0.1*SCREEN_WIDTH)
					al_set_mouse_xy(display,0.9*SCREEN_WIDTH,
							event.mouse.y);
				else if(event.mouse.x>0.9*SCREEN_WIDTH)
					al_set_mouse_xy(display,
							0.1*SCREEN_WIDTH,
							event.mouse.y);
				else{
					player.hor_angle +=
						(event.mouse.dx)*2*
						ALLEGRO_PI/SCREEN_WIDTH;
					player.vert_angle += 
						(event.mouse.dy)*
						ALLEGRO_PI/SCREEN_HEIGHT;
					if(player.vert_angle>ALLEGRO_PI/2)
						player.vert_angle=ALLEGRO_PI/2;
					if(player.vert_angle<-ALLEGRO_PI/2)
						player.vert_angle=-ALLEGRO_PI/2;
		      		}
				break;
		  	case ALLEGRO_EVENT_KEY_UP:
				key[event.keyboard.keycode] &= KEY_RELEASED;
				break;
		   	case ALLEGRO_EVENT_KEY_DOWN:
				key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
		      		switch (event.keyboard.keycode) {
		  	    		case ALLEGRO_KEY_ESCAPE:
		  	       			quit = true;
		  	       			break;
		  	    		case ALLEGRO_KEY_F11:
		  	       			fullscreen = !fullscreen;
						al_set_display_flag(display,
							ALLEGRO_FULLSCREEN_WINDOW,
		  			       		fullscreen);
						break;
					case ALLEGRO_KEY_1:
						block_selection=GRASS_BLOCK;
						break;
					case ALLEGRO_KEY_2:
						block_selection=DIRT_BLOCK;
						break;
					case ALLEGRO_KEY_3:
						block_selection=COBBLESTONE_BLOCK;
						break;
					case ALLEGRO_KEY_4:
						block_selection=OAK_PLANKS_BLOCK;
						break;
					case ALLEGRO_KEY_5:
						block_selection=OAK_LOG_BLOCK;
						break;
					case ALLEGRO_KEY_6:
						block_selection=STONE_BLOCK;
						break;
					case ALLEGRO_KEY_K:
						player.vert_speed=0;
						flying=!flying;
						break;
					case ALLEGRO_KEY_SPACE:
						if(!flying){
							if(player.falling)break;
							player.falling=true;
							player.vert_speed=0.3;
						}
						break;
		      		}
		      		break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		      		Direction dir;
				unsigned int block_index = ray_block_from_player(&dir,player,player.chunk_index);
				if(block_index==UINT_MAX)break;
				switch(event.mouse.button){
					case 2:
						if(dir==NoDirection)break;
						Pos block = INDEX_TO_POS(&world,block_index);
						int chunk_index = getChunk(&world,adjacentPos(block,dir)/*(ADD_VEC(block, dirVectors[dir]))*/,block_index/MAX_CHUNK_BlOCKS);
						ChunkPos p =worldl(&world)[chunk_index].pos;
						block_index = POS_TO_INDEX((&world),adjacentPos(block,dir), block_index/MAX_CHUNK_BlOCKS);
						setBlock(&world,block_index,chunk_index,block_selection);
						break;
					case 1://break block
						setBlock(&world,block_index,block_index/MAX_CHUNK_BlOCKS,AIR);
						break;
				}
				break;
			case ALLEGRO_EVENT_TIMER:
				redraw = true;
		      		if(key[ALLEGRO_KEY_W])move(SPEED/FRAMERATE,&player,0,!flying);
		  		else if(key[ALLEGRO_KEY_S])
		  			move(SPEED/FRAMERATE,&player,ALLEGRO_PI,!flying);
		  		else if(key[ALLEGRO_KEY_D]){
		  			move(SPEED/FRAMERATE,&player,ALLEGRO_PI/2,!flying);
				}
		  		else if(key[ALLEGRO_KEY_A])
		  			move(SPEED/FRAMERATE,&player,-ALLEGRO_PI/2,!flying);
		      		if(key[ALLEGRO_KEY_SPACE]&&flying){
					player.pos.y+=20.0/FRAMERATE;
		      		}
		      		if(key[ALLEGRO_KEY_LSHIFT]||key[ALLEGRO_KEY_LSHIFT]){
					if(flying)player.pos.y-=20.0/FRAMERATE;
		      		}
		      		int i=0;
		      		while(i < ALLEGRO_KEY_MAX){
		      			key[i] &= KEY_SEEN;
		      			i++;
		      		}
				break;
		   	case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
		   		background = true;
		   		al_acknowledge_drawing_halt(display);
		   		al_stop_timer(timer);
		   		break;
		   	case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
		   		background = false;
		   		al_acknowledge_drawing_resume(display);
		    		al_start_timer(timer);
		   		break;
		}
		int i = 0;
		if (!background && redraw && al_is_event_queue_empty(queue)) {
			set_perspective_transform(al_get_backbuffer(display),player);
			
			
			al_set_target_backbuffer(display);
			al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
			al_clear_to_color(al_map_rgb_f(0, 0, 0));
			al_clear_depth_buffer(1000);
			draw_world(&world,&player);
			
			display_2d = al_create_sub_bitmap(al_get_backbuffer(display), 0, 0,
				al_get_display_width(display), al_get_display_height(display));
			al_set_target_bitmap(display_2d);
			al_draw_rectangle(al_get_bitmap_width(display_2d)/2-10,
			       	 al_get_bitmap_height(display_2d)/2-10,
			       		 al_get_bitmap_width(display_2d)/2+10,
			       	 al_get_bitmap_height(display_2d)/2+10, al_map_rgb_f(1,
			       		 1, 1), 2);
			char info_str[100];
			const char* walking_or_flyingStr;
			if(flying)walking_or_flyingStr=flyingStr;
			else walking_or_flyingStr=walkingStr;
			sprintf(info_str, "Position: %f, %f, %f, (%s mode)", player.pos.x,
					player.pos.y, player.pos.z,walking_or_flyingStr);
         		al_draw_text(font, al_map_rgb_f(1, 1, 1), 0, 0, 0,
                      		info_str);
			
			al_destroy_bitmap(display_2d);
			
			al_flip_display();
			redraw = false;
		}
	}
   	return 0;
}
// vim: cc=100 
