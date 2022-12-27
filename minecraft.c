#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "list.h"

#define FRAMERATE 60
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600



typedef struct{
	float x,y,z;
}Pos;
typedef Pos Vec;
#define ADD_VEC(v,w) {v.x+w.x,v.y+w.y,v.z+w.z}

typedef struct{
	float hor_angle;
	float vert_angle;
	Pos pos;
}Player;


typedef enum {DIRT=0, GRASS_BLOCK_SIDE=1, GRASS_BLOCK_TOP=2, COBBLESTONE=3,
	OAK_PLANKS=4, OAK_LOG=5, OAK_LOG_TOP=6} Texture;
char* texturesStr[] = {"dirt.png", "grass_block_side.png",
	"grass_block_top.png", "cobblestone.png", "oak_planks.png",
	"oak_log.png", "oak_log_top.png"};
#define LEN_TEXTURES  sizeof(texturesStr)/sizeof(char*)
ALLEGRO_BITMAP* texturesBMP[LEN_TEXTURES];

typedef struct{
		  int r,g,b;
}Color;

typedef struct{
		  Texture texture;
		  Color color;
}BlockSidePreProcessing;

//0:top, 1:bottem, 2:north, 3:west, 4:east, 5:south
typedef BlockSidePreProcessing BlockTypePreProcessing[6];

typedef struct{
		  ALLEGRO_BITMAP* texture;
		  ALLEGRO_COLOR color;
}BlockSide;

//0:top, 1:bottem, 2:north, 3:west, 4:east, 5:south
typedef BlockSide BlockType[6];
#define WHITE {255,255,255}

typedef enum{GRASS_BLOCK=0, DIRT_BLOCK=1, COBBLESTONE_BLOCK=2,
	OAK_PLANKS_BLOCK=3, OAK_LOG_BLOCK=4}BlockTypes;

BlockTypePreProcessing blockTypesPreProcessing[] =
{	
	{/*grass_block*/{GRASS_BLOCK_TOP, {124,  189,  1}}, {DIRT, WHITE},
	{GRASS_BLOCK_SIDE, WHITE}, {GRASS_BLOCK_SIDE, WHITE},
	{GRASS_BLOCK_SIDE, WHITE}, {GRASS_BLOCK_SIDE, WHITE}}, 

	{/*dirt_block*/{DIRT, WHITE}, {DIRT, WHITE}, {DIRT, WHITE}, {DIRT, WHITE},
	{DIRT, WHITE}, {DIRT, WHITE}}, 

	{/*cobblestone*/{COBBLESTONE, WHITE}, {COBBLESTONE, WHITE},
		{COBBLESTONE, WHITE}, {COBBLESTONE, WHITE}, {COBBLESTONE, WHITE},
		{COBBLESTONE, WHITE}}, 
	{/*oak_planks*/{OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE},
	{OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE}}, 

	{/*oak_log*/{OAK_LOG_TOP, WHITE}, {OAK_LOG_TOP, WHITE}, {OAK_LOG, WHITE},
	{OAK_LOG, WHITE}, {OAK_LOG, WHITE}, {OAK_LOG, WHITE}}, 
};
#define LEN_BLOCK_TYPES sizeof(blockTypesPreProcessing)/sizeof(BlockTypePreProcessing)

BlockType blockTypes[LEN_BLOCK_TYPES];
	
typedef struct{
	int type_index;
	Pos pos;
}Block;

Block blocks[] ={{COBBLESTONE_BLOCK,{0,0,0}}};
List block_list;

#define LEN_BLOCKS sizeof(blocks)/sizeof(Block)
#define LEN_BLOCK_LIST block_list.used/sizeof(Block)


#define SIDE_COLOR(dir) (blockTypes[block.type_index][dir]).color

int is_block(Pos pos){
   	for(int i=0;i<LEN_BLOCK_LIST;i++){
		Block b = ((Block*)block_list.l)[i];
		if(	floorf(b.pos.x)==floorf(pos.x) &&
		  	floorf(b.pos.y)==floorf(pos.y) &&
			floorf(b.pos.z)==floorf(pos.z))return(i);
	}
	return INT_MAX;
}

typedef enum{Top=0,Bottem=1,West=2,East=3,South=4,North=5,NoDirection} Direction;
Vec dirVectors[6]={{0,1,0},{0,-1,0},{-1,0,0},{1,0,0},{0,0,-1},{0,0,1}};

bool lessThan(Direction dir, Pos p1, Pos p2){
	switch(dir){
		case Top: return(p1.y<p2.y);
		case Bottem: return(p1.y>p2.y);
		case West: return(p1.x>p2.x);
		case East: return(p1.x<p2.x);
		case South: return(p1.z>p2.z);
		case North: return(p1.z<p2.z);
	}
}

static void draw_block(Block block,Player* player)
{
	ALLEGRO_COLOR north_c=(blockTypes[block.type_index][2]).color;
	ALLEGRO_VERTEX faces[6][4]={{
	        //top
	/*   x   y   z   u   v  c  */
	   { 1,  1, 0,  0,  0, SIDE_COLOR(Top)},//1
	   { 1,  1,  1, 16,  0, SIDE_COLOR(Top)},//2
	   {0,  1, 0,  0, 16, SIDE_COLOR(Top)},//0
	   {0,  1,  1, 16, 16, SIDE_COLOR(Top)},//3
	},{
	        //bottem
	/*   x   y   z   u   v  c  */
	   {0, 0, 0,  0,  0, SIDE_COLOR(Bottem)},//4
	   {0, 0,  1, 16,  0, SIDE_COLOR(Bottem)},//7
	   { 1, 0, 0,  0, 16, SIDE_COLOR(Bottem)},//5
	   { 1, 0,  1, 16, 16, SIDE_COLOR(Bottem)},//6
	},{
	        //west
	/*   x   y   z   u   v  c  */
	   {0,  1, 0,  0,  0, SIDE_COLOR(West)},//0
	   {0, 0, 0,  0, 16, SIDE_COLOR(West)},//4
	   {0,  1,  1, 16,  0, SIDE_COLOR(West)},//3
	   {0, 0,  1, 16, 16, SIDE_COLOR(West)},//7
	},{
	        //east
	/*   x   y   z   u   v  c  */
	   { 1, 0, 0,16,16, SIDE_COLOR(East)},//5
	   { 1,  1, 0,16,0, SIDE_COLOR(East)},//1
	   { 1, 0,  1,0,16, SIDE_COLOR(East)},//6
	   { 1,  1,  1,0,0, SIDE_COLOR(East)},//2
	},{
	        //south
	/*   x   y   z   u   v  c  */
	   {0,  1,  1,  0,  0, SIDE_COLOR(South)},//3
	   { 1,  1,  1, 16,  0, SIDE_COLOR(South)},//2
	   {0, 0,  1,  0, 16, SIDE_COLOR(South)},//7
	   { 1, 0,  1, 16, 16, SIDE_COLOR(South)},//6
	},{
	        //north
	/*   x   y   z   u   v  c  */
	   {0,  1, 0,  0,  0, SIDE_COLOR(North)},//0
	   { 1,  1, 0, 16,  0, SIDE_COLOR(North)},//1
	   {0, 0, 0,  0, 16, SIDE_COLOR(North)},//4
	   { 1, 0, 0, 16, 16, SIDE_COLOR(North)},//5
	}};
	
	
	int indices[6] = {
	   0,1,3,
	   0,2,3
	};
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_translate_transform_3d(&t, block.pos.x, block.pos.y, -block.pos.z-1);
	al_use_transform(&t);
	for(int i=0;i<6;i++){
		//If the side shouldn't be visible there is no point
		//drawing it
		if(lessThan(i,player->pos,block.pos))continue;

		//only draw the face if there is no face next to it
		Pos adjacent_block_pos = ADD_VEC(block.pos,dirVectors[i]);
		if(is_block(adjacent_block_pos)==INT_MAX)
		al_draw_indexed_prim(faces[i], NULL,
				(blockTypes[block.type_index][i]).texture,
				indices, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
   }
}

void draw_map(Player* p)
{
   for(int i=0;i<LEN_BLOCK_LIST;i++){
	   draw_block(((Block*)block_list.l)[i],p);
   }
}

void set_perspective_transform(ALLEGRO_BITMAP* bmp,Player player)
{
   ALLEGRO_TRANSFORM p;
   float aspect_ratio = (float)al_get_bitmap_height(bmp) / al_get_bitmap_width(bmp);
   al_set_target_bitmap(bmp);
   al_identity_transform(&p);
   al_translate_transform_3d(&p,-player.pos.x,-player.pos.y,player.pos.z);
	al_rotate_transform_3d(&p,0,1,0,player.hor_angle);
	al_rotate_transform_3d(&p,1,0,0,player.vert_angle);
   al_perspective_transform(&p, -1, aspect_ratio, 1, 1, -aspect_ratio, 1000);
   al_use_projection_transform(&p);
}

void loadBlocks(){
	bool success = al_change_directory("pics");
	if(!success){
		printf("Could not change directory");
		exit(1);
	}

	for(int i=0;i<LEN_TEXTURES;i++){
		texturesBMP[i]=al_load_bitmap(texturesStr[i]);
		if(texturesBMP[i]==NULL){
			printf("Could not load %s\n",texturesStr[i]);
			exit(1);
		}
	}
	al_change_directory("..");
	for(int i=0;i<LEN_BLOCK_TYPES;i++){
		for(int j=0;j<6;j++){
			Texture t = blockTypesPreProcessing[i][j].texture;
			Color c = blockTypesPreProcessing[i][j].color;
			blockTypes[i][j].texture = texturesBMP[t];
			blockTypes[i][j].color = al_map_rgb(c.r,c.g,c.b);
		}
	}
}


void move(float step, Player* p,float angle){
	float x = cos(p->hor_angle+angle);
	float y = sin(p->hor_angle+angle);
	Pos new_pos = p->pos;
	new_pos.z+=step*x;
	new_pos.x+=step*y;
	if(is_block(new_pos)==INT_MAX)
		p->pos=new_pos;
}


Direction hitDirectionBlock(Pos prev_pos, Block b){
	if(prev_pos.y>b.pos.y+1)return Top;
	else if(prev_pos.y<b.pos.y)return Bottem;
	else if(prev_pos.x<b.pos.x)return West;
	else if(prev_pos.x>b.pos.x+1)return East;
	else if(prev_pos.z<b.pos.z)return South;
	else if(prev_pos.z>b.pos.z+1)return North;
	else return NoDirection;
}


int ray_block(Direction* dir,Vec v,Pos pos){
	for(int i=0;i<100;i++){
		Pos prev_pos = pos;
		pos =(Pos)ADD_VEC(v,pos);
		int ret;
		if((ret=is_block(pos))!=INT_MAX){
			if(dir)*dir=hitDirectionBlock(prev_pos,
					((Block*)block_list.l)[ret]);
			return(ret);
		}
	}
	return(INT_MAX);
}

int ray_block_from_player(Direction* dir,Player p){
	Vec v = {0.5*sin(p.hor_angle)*cos(p.vert_angle), 0.5*sin(-p.vert_angle),
		0.5*cos(p.hor_angle)*cos(p.vert_angle)};
	//printf("vec:(%f,%f,%f)\n",v.x,v.y,v.z);
	//printf("pos:(%f,%f,%f)\n",p.pos.x,p.pos.y,p.pos.z);
	return(ray_block(dir, v,p.pos));
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
	
	
	Player player={0,0,{0,0,0}};
	bool redraw = false;
	bool quit = false;
	bool fullscreen = false;
	bool background = false;
	int display_flags = ALLEGRO_RESIZABLE;
	
	
	if (!al_init()) {
		printf("Could not init Allegro.\n");
		exit(1);
	}
	al_init_image_addon();
	loadBlocks();
	
	ALLEGRO_FILE* f = al_fopen("world.bin","r");
	if(f){
		int len_blocks;
		al_fread(f,&len_blocks,sizeof(int));
		Block* block_listt = malloc(sizeof(Block)*len_blocks);
		al_fread(f,block_listt,sizeof(Block)*len_blocks);
		block_list.available=len_blocks;
		block_list.used=len_blocks;
		block_list.l=block_listt;
	}else block_list = list_init_with(sizeof(Block)*LEN_BLOCKS,blocks);
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
	
	BlockTypes block_selection=0;
	
	al_start_timer(timer);
	while (!quit) {
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
					case ALLEGRO_KEY_O://save the world
						ALLEGRO_FILE* f = 
							al_fopen("world.bin","w");
						al_fwrite(f,&(block_list.used),
							sizeof(int));
						al_fwrite(f,block_list.l,
							sizeof(Block)*block_list.used);
						al_fclose(f);
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
		      		}
		      		break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		      		Direction dir;
				int block_index = ray_block_from_player(&dir,player);
				if(block_index==INT_MAX)break;
				switch(event.mouse.button){
					case 2://place block
						Pos new_pos=((Block*)block_list.l)
							[block_index].pos;
						if(dir==NoDirection)break;
						switch(dir){
							case(Top):
								new_pos.y++;
								break;
							case(Bottem):
								new_pos.y--;
								break;
							case(West):
								new_pos.x--;
								break;
							case(East):
								new_pos.x++;
								break;
							case(South):
								new_pos.z--;
								break;
							case(North):
								new_pos.z++;
								break;
						}
						list_append(&block_list,sizeof(Block));
						Block new_block =
							{block_selection,new_pos};
						((Block*)block_list.l)[LEN_BLOCK_LIST-1]
							=new_block;
						break;
					case 1://break block
						list_pop(&block_list,
								sizeof(Block)*block_index,
								sizeof(Block));
						break;
				}
				break;
			case ALLEGRO_EVENT_TIMER:
				redraw = true;
		      		if(key[ALLEGRO_KEY_W])move(20.0/FRAMERATE,&player,0);
		  		else if(key[ALLEGRO_KEY_S])
		  			move(20.0/FRAMERATE,&player,ALLEGRO_PI);
		  		else if(key[ALLEGRO_KEY_D])
		  			move(20.0/FRAMERATE,&player,ALLEGRO_PI/2);
		  		else if(key[ALLEGRO_KEY_A])
		  			move(20.0/FRAMERATE,&player,-ALLEGRO_PI/2);
		      		if(key[ALLEGRO_KEY_SPACE]){
		  			Pos new_pos = player.pos;
		      			new_pos.y+=20.0/FRAMERATE;
		  			if(is_block(new_pos)==INT_MAX)player.pos=new_pos;
		      		}
		      		if(key[ALLEGRO_KEY_LSHIFT]||key[ALLEGRO_KEY_LSHIFT]){
		  			Pos new_pos = player.pos;
		      			new_pos.y-=20.0/FRAMERATE;
		  			if(is_block(new_pos)==INT_MAX)player.pos=new_pos;
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
			draw_map(&player);
			
			display_2d = al_create_sub_bitmap(al_get_backbuffer(display), 0, 0,
				al_get_display_width(display), al_get_display_height(display));
			al_set_target_bitmap(display_2d);
			al_draw_rectangle(al_get_bitmap_width(display_2d)/2-10,
			       	 al_get_bitmap_height(display_2d)/2-10,
			       		 al_get_bitmap_width(display_2d)/2+10,
			       	 al_get_bitmap_height(display_2d)/2+10, al_map_rgb_f(1,
			       		 1, 1), 2);
			
			al_destroy_bitmap(display_2d);
			
			al_flip_display();
			redraw = false;
		}
	}
   	return 0;
}
// vim: cc=100 
