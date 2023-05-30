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
#include "chunk.h"
#include "block.h"
#include "world.h"
#include "minecraft.h"

#define FRAMERATE 30.0
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define WALKING_SPEED 5.0
#define FLYING_SPEED 20.0
#define GRAVITY -0.5





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



bool isValidPlayerPos(List* world, Pos pos, unsigned int chunk_index){
	Pos pos_low=pos;
	pos_low.y--;
	return(is_block(world,pos_low,chunk_index)==UINT_MAX
					//Checks if the lower part of the player's body is
					//in a valid position
			&& is_block(world,pos,chunk_index)==UINT_MAX);
}


//TODO: Check if it is necessary
void getVisibleChunks(List* world, int* visibleChunks,Player* p){
	ChunkPos playerChunkPos = POS_TO_CHUNK_POS(p->pos);
	for(int i=0;i<(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1);i++){
		ChunkPos chunkPos = {	playerChunkPos.x+(-WORLD_CHUNKS+i/(2*WORLD_CHUNKS+1)),
					playerChunkPos.z+(-WORLD_CHUNKS+i%(2*WORLD_CHUNKS+1))};
		visibleChunks[i]=getChunk(world,chunkPos,p->chunk_index);
	}
}


void move(Minecraft* minecraft, float step, Player* p,float angle,bool checkValid){
	int prevChunkIndex = p->chunk_index;

	float x = cos(p->hor_angle+angle);
	float y = sin(p->hor_angle+angle);
	Pos new_pos = p->pos;
	Pos new_pos_extra = p->pos;
	new_pos.z+=step*x;
	new_pos.x+=step*y;
	
	new_pos_extra.z+=(step+0.1)*x;
	new_pos_extra.x+=(step+0.1)*y;

	//check if the chunk index is still correct
	unsigned int new_chunk_index = getChunk(&(minecraft->world),POS_TO_CHUNK_POS(new_pos), p->chunk_index);

	if(!checkValid ||  isValidPlayerPos(&(minecraft->world),new_pos_extra,new_chunk_index)){
		p->pos=new_pos;
		p->chunk_index=new_chunk_index;
	}
	if(prevChunkIndex!=p->chunk_index){
		loadChunks(&(minecraft->world),minecraft->visibleChunks,*p);
		//getVisibleChunks(visibleChunks,p);

		//reCreateWorldMesh(&world,p);
	}
}

unsigned int ray_block(List* world,Direction* dir,Vec v,Pos pos,unsigned int chunk_index){
	for(unsigned int i=0;i<100;i++){
		Pos prev_pos = pos;
		pos =(Pos)ADD_VEC(v,pos);
		unsigned int ret;
		if((ret=is_block(world,pos,chunk_index))!=UINT_MAX){
			if(dir)*dir=hitDirectionBlock(prev_pos,
					INDEX_TO_POS(world,ret));
			return(ret);
		}
	}
	return(UINT_MAX);
}

unsigned int ray_block_from_player(List* world,Direction* dir,Player p,unsigned int chunk_index){
	Vec v = {0.5*sin(p.hor_angle)*cos(p.vert_angle), 0.5*sin(-p.vert_angle),
		0.5*cos(p.hor_angle)*cos(p.vert_angle)};
	return(ray_block(world,dir, v,p.pos,chunk_index));
}

//Gives the distance of x to [a,b] where a<=b
float distanceInterval(float x,float a,float b){
	if(x<=a)return(a-x);
	else if(b<=x)return(x-b);
	else return(0);
}
extern ALLEGRO_DISPLAY* display;

extern int screen_width;
extern int screen_height;


//TODO: Create this function
//void drawText(Minecraft* minecraft){
//	
//}

void drawMinecraft(Minecraft* minecraft){
	const char walkingStr[] ="Walking";
	const char flyingStr[] = "Flying";
   	ALLEGRO_FONT* font;
	font = al_create_builtin_font();
	ALLEGRO_BITMAP* display_2d;
	display_2d = al_create_sub_bitmap(al_get_backbuffer(display), 0, 0,
	     	   screen_width, screen_height);
	al_set_target_backbuffer(display);
	al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
	al_clear_to_color(al_map_rgb_f(0, 0, 0));
	al_clear_depth_buffer(1000);
	
	set_perspective_transform(al_get_backbuffer(display),minecraft->player);
	draw_world(&(minecraft->world),minecraft->visibleChunks,&(minecraft->player));
	
	display_2d = al_create_sub_bitmap(al_get_backbuffer(display), 0, 0,
		al_get_display_width(display), al_get_display_height(display));
	al_set_target_bitmap(display_2d);
	
	int width2d = al_get_bitmap_width(display_2d);
	int height2d = al_get_bitmap_height(display_2d);

	//draw hotbar
	int hotbarHeight = width2d/2/9;
	ALLEGRO_BITMAP* icon;
	al_set_target_bitmap(display_2d);
	
	ALLEGRO_TRANSFORM I;
	al_identity_transform(&I);
	al_use_transform(&I);

	al_draw_rectangle(width2d/2-10,height2d/2-10,
			width2d/2+10, height2d/2+10, al_map_rgb_f(1, 1, 1), 2);
	
	drawHotbar(width2d/4,height2d-hotbarHeight,width2d/2,hotbarHeight,minecraft->hotbarSelect,minecraft->hotbar);
	
	char info_str[100];
	const char* walking_or_flyingStr;
	if(minecraft->player.flying)walking_or_flyingStr=flyingStr;
	else walking_or_flyingStr=walkingStr;
	sprintf(info_str, "Position: %f, %f, %f, (%s mode)", minecraft->player.pos.x,
			minecraft->player.pos.y, minecraft->player.pos.z,walking_or_flyingStr);
	al_draw_text(font, al_map_rgb_f(1, 1, 1), 0, 0, 0,
		info_str);
	
	al_destroy_bitmap(display_2d);
	
}

void handleMinecraft(Minecraft* minecraft, ALLEGRO_EVENT* event){

	//TODO: See if this can be only run if it is a ALLEGRO_EVENT_TIMER
	if(!minecraft->player.flying){
	Pos new_pos = minecraft->player.pos;
	new_pos.y+=minecraft->player.vert_speed/FRAMERATE;
	Pos foot;
	foot=new_pos;
	foot.y-=1.8;
	unsigned int b= is_block(&(minecraft->world),foot,minecraft->player.chunk_index);
	if(b==UINT_MAX/*TODO: Give a name that is easier to understand*/){
		Pos prev_block_pos;
		if(!minecraft->player.falling)
			prev_block_pos = INDEX_TO_POS(&(minecraft->world),minecraft->player.standing_on);
		if(	minecraft->player.standing_on==UINT_MAX ||
			WORLD_BLOCK_INDEX(&(minecraft->world),minecraft->player.standing_on)==AIR
			|| minecraft->player.falling
			//check if the player is still standing on a block
			|| distanceInterval(minecraft->player.pos.x,prev_block_pos.x,
				prev_block_pos.x+1.0)>0.5 
			|| distanceInterval(minecraft->player.pos.z,prev_block_pos.z,
					prev_block_pos.z+1.0)>0.5){


			minecraft->player.falling=true;
			minecraft->player.pos.y+=minecraft->player.vert_speed;
		}
	}
	else{
		minecraft->player.standing_on=b;
		minecraft->player.falling=false;
		minecraft->player.pos.y=ceilf(minecraft->player.pos.y-1.8)+1.8;
		minecraft->player.vert_speed=0;
	}
	if(minecraft->player.falling)minecraft->player.vert_speed+=GRAVITY/FRAMERATE;
	}
	
		
#define KEY_SEEN     1
#define KEY_RELEASED 2
	switch (event->type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			exit(0);
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			al_acknowledge_resize(display);
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
			//wraps the mouse around if necessary
			if(event->mouse.x<0.1*SCREEN_WIDTH)
				al_set_mouse_xy(display,0.9*SCREEN_WIDTH,
						event->mouse.y);
			else if(event->mouse.x>0.9*SCREEN_WIDTH)
				al_set_mouse_xy(display,
						0.1*SCREEN_WIDTH,
						event->mouse.y);
			else{
				minecraft->player.hor_angle +=
					(event->mouse.dx)*2*
					ALLEGRO_PI/SCREEN_WIDTH;
				minecraft->player.vert_angle += 
					(event->mouse.dy)*
					ALLEGRO_PI/SCREEN_HEIGHT;
				if(minecraft->player.vert_angle>ALLEGRO_PI/2)
					minecraft->player.vert_angle=ALLEGRO_PI/2;
				if(minecraft->player.vert_angle<-ALLEGRO_PI/2)
					minecraft->player.vert_angle=-ALLEGRO_PI/2;
	      		}
			minecraft->hotbarSelect-=event->mouse.dz;
			if(minecraft->hotbarSelect<0)minecraft->hotbarSelect=0;
			if(minecraft->hotbarSelect==9)minecraft->hotbarSelect=8;
			minecraft->block_selection=minecraft->hotbarSelect;
			break;
	  	case ALLEGRO_EVENT_KEY_UP:
			minecraft->key[event->keyboard.keycode] &= KEY_RELEASED;
			break;
	   	case ALLEGRO_EVENT_KEY_DOWN:
			minecraft->key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
	      		switch (event->keyboard.keycode) {
				//TODO: Ensure that this is handled
				//in the main loop
	  	    		//case ALLEGRO_KEY_ESCAPE:
				//	exit(0);
	  	    		//case ALLEGRO_KEY_F11:
	  	       		//	fullscreen = !fullscreen;
				//	al_set_display_flag(display,
				//		ALLEGRO_FULLSCREEN_WINDOW,
	  			//       		fullscreen);
				//	break;
				case ALLEGRO_KEY_1:
					minecraft->block_selection=GRASS_BLOCK;
					minecraft->hotbarSelect = 0;
					break;
				case ALLEGRO_KEY_2:
					minecraft->block_selection=DIRT_BLOCK;
					minecraft->hotbarSelect = 1;
					break;
				case ALLEGRO_KEY_3:
					minecraft->block_selection=COBBLESTONE_BLOCK;
					minecraft->hotbarSelect = 2;
					break;
				case ALLEGRO_KEY_4:
					minecraft->block_selection=OAK_PLANKS_BLOCK;
					minecraft->hotbarSelect = 3;
					break;
				case ALLEGRO_KEY_5:
					minecraft->block_selection=OAK_LOG_BLOCK;
					minecraft->hotbarSelect = 4;
					break;
				case ALLEGRO_KEY_6:
					minecraft->block_selection=STONE_BLOCK;
					minecraft->hotbarSelect = 5;
					break;
				case ALLEGRO_KEY_7:
					minecraft->block_selection=OAK_LEAVES_BLOCK;
					minecraft->hotbarSelect = 6;
					break;
				case ALLEGRO_KEY_8:
					minecraft->block_selection=GLASS_BLOCK;
					minecraft->hotbarSelect = 7;
					break;
				case ALLEGRO_KEY_9:
					minecraft->block_selection=WATER_BLOCK;
					minecraft->hotbarSelect = 8;
					break;
				case ALLEGRO_KEY_K:
					minecraft->player.vert_speed=0;
					minecraft->player.flying=!minecraft->player.flying;
					break;
				case ALLEGRO_KEY_O:
					FILE* f= fopen("world.bin","w");
					saveWorld(f,&(minecraft->world));
					fclose(f);
					break;
				case ALLEGRO_KEY_SPACE:
					if(!minecraft->player.flying){
						if(minecraft->player.falling)break;
						minecraft->player.falling=true;
						minecraft->player.vert_speed=0.15;
					}
					break;
	      		}
	      		break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
	      		Direction dir;
			unsigned int block_index = ray_block_from_player(&(minecraft->world),&dir,minecraft->player,
					minecraft->player.chunk_index);
			if(block_index==UINT_MAX)break;
			Pos block = INDEX_TO_POS(&(minecraft->world),block_index);
			switch(event->mouse.button){
				case 2:
					if(dir==NoDirection)break;
					int chunk_index = getChunk(&(minecraft->world),
						POS_TO_CHUNK_POS(adjacentPos(block,dir)),
						block_index/MAX_CHUNK_BlOCKS
					);
					ChunkPos p =worldl(&(minecraft->world))[chunk_index].pos;
					block_index = POS_TO_INDEX((&(minecraft->world)),
						adjacentPos(block,dir), 
						chunk_index
					);
					int player_block_index = POS_TO_INDEX(
						(&(minecraft->world)),minecraft->player.pos,chunk_index);
					
					Pos feet_pos = adjacentPos(minecraft->player.pos,Bottem);

					int feet_block_index = POS_TO_INDEX(
						(&(minecraft->world)),feet_pos,chunk_index);
					if(player_block_index==block_index
						||feet_block_index==block_index)break;

					setBlock(&(minecraft->world),block_index,chunk_index,
							minecraft->block_selection);
					break;
				case 1://break block
					setBlock(&(minecraft->world),block_index,
							block_index/MAX_CHUNK_BlOCKS,AIR);
					break;
			}
			////ChunkPos playerChunkPos = POS_TO_CHUNK_POS(player.pos);
			//ChunkPos blockChunkPos =POS_TO_CHUNK_POS(block);
			////int i = (blockChunkPos.x-playerChunkPos.x+WORLD_CHUNKS)*
			////	(2*WORLD_CHUNKS+1)+
			////	(blockChunkPos.z-playerChunkPos.z+WORLD_CHUNKS);
			//int block_chunk = getChunk(&world,blockChunkPos,player.chunk_index);
			//
			//Chunk* chunk = &(worldl(&world)[block_chunk]);
			////List* mesh = &(worldl(&world)[block_chunk].mesh);

			break;
		case ALLEGRO_EVENT_TIMER:
			float speed;
			if(minecraft->player.flying)speed = FLYING_SPEED;
			else speed = WALKING_SPEED;
	      		if(minecraft->key[ALLEGRO_KEY_W]){
				move(minecraft,speed/FRAMERATE,&(minecraft->player),0,!minecraft->player.flying);
			}
	  		else if(minecraft->key[ALLEGRO_KEY_S])
	  			move(minecraft,speed/FRAMERATE,&(minecraft->player),ALLEGRO_PI,!minecraft->player.flying);
	  		else if(minecraft->key[ALLEGRO_KEY_D]){
	  			move(minecraft,speed/FRAMERATE,&(minecraft->player),ALLEGRO_PI/2,!minecraft->player.flying);
			}
	  		else if(minecraft->key[ALLEGRO_KEY_A])
	  			move(minecraft,speed/FRAMERATE,&(minecraft->player),-ALLEGRO_PI/2,!minecraft->player.flying);
	      		if(minecraft->key[ALLEGRO_KEY_SPACE]&&minecraft->player.flying){
				minecraft->player.pos.y+=20.0/FRAMERATE;
	      		}
	      		if(minecraft->key[ALLEGRO_KEY_LSHIFT]||minecraft->key[ALLEGRO_KEY_LSHIFT]){
				if(minecraft->player.flying)minecraft->player.pos.y-=20.0/FRAMERATE;
	      		}
	      		int i=0;
	      		while(i < ALLEGRO_KEY_MAX){
	      			minecraft->key[i] &= KEY_SEEN;
	      			i++;
	      		}
			break;
	}
}
		

void initPlayer(Player* p){
	p->hor_angle = 0;
	p->vert_angle = 0;
	p->pos.x=0;
	p->pos.y=25;
	p->pos.z=0;
	p->chunk_index=UINT_MAX;
	p->flying=false;
	p->falling=true;
	p->vert_speed=0;
	p->standing_on=UINT_MAX;
}

void initMinecraft(Minecraft* minecraft){
	initPlayer(&(minecraft->player));
	al_hide_mouse_cursor(display);
	al_grab_mouse(display);
	al_set_mouse_xy(display,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);

	FILE* f = fopen("world.bin","r");
	if(f){
		loadWorld(f,&(minecraft->world));
	}else{
		minecraft->world= list_init(0);
	}
	minecraft->hotbarSelect = 0;
	for(int i=0;i<9;i++){
		minecraft->hotbar[i]=i;
	}
	minecraft->block_selection=DIRT_BLOCK;
	loadChunks(&(minecraft->world),minecraft->visibleChunks,minecraft->player);

	memset(minecraft->key, 0, sizeof(minecraft->key));
	
}
// vim: cc=100 
