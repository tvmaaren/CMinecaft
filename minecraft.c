#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "list.h"

#define FRAMERATE 60.0
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SIZE_BLOCK 8

//Amount of Chunks that should be loaded in every
//direction
#define WORLD_CHUNKS 1


typedef struct{
	float x,y,z;
}Pos;
typedef Pos Vec;

#define ADD_VEC(v,w) (Pos){v.x+w.x,v.y+w.y,v.z+w.z}
#define SCALAR_MUL_VEC(l,v) (Vec){l*v.x,l*v.y,l*v.z}

#define SPEED 5.0
#define GRAVITY -0.5
typedef struct{
	float hor_angle;
	float vert_angle;
	Pos pos;
	unsigned int chunk_index;
	bool falling;
	float vert_speed;//only used when falling
}Player;


typedef enum {DIRT=0, GRASS_BLOCK_SIDE=1, GRASS_BLOCK_TOP=2, COBBLESTONE=3,
	OAK_PLANKS=4, OAK_LOG=5, OAK_LOG_TOP=6,STONE=7} Texture;
char* texturesStr[] = {"dirt.png", "grass_block_side.png",
	"grass_block_top.png", "cobblestone.png", "oak_planks.png",
	"oak_log.png", "oak_log_top.png","stone.png"};
#define LEN_TEXTURES  sizeof(texturesStr)/sizeof(char*)
ALLEGRO_BITMAP* texturesBMP[LEN_TEXTURES];

typedef struct{
		  uint8_t r,g,b;
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
typedef BlockSide BlockTypes[6];
#define WHITE {255,255,255}

typedef enum{AIR=UINT_MAX, GRASS_BLOCK=0, DIRT_BLOCK=1, COBBLESTONE_BLOCK=2,
	OAK_PLANKS_BLOCK=3, OAK_LOG_BLOCK=4, STONE_BLOCK=5}BlockType;

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

	{/*stone_block*/{STONE,WHITE}, {STONE,WHITE}, {STONE,WHITE}, {STONE,WHITE},
	{STONE, WHITE}, {STONE, WHITE}}
};
#define LEN_BLOCK_TYPES sizeof(blockTypesPreProcessing)/sizeof(BlockTypePreProcessing)

BlockTypes blockTypes[LEN_BLOCK_TYPES];


#define CHUNK_WIDTH 		16 //Depth and with of a chunk
#define MAX_CHUNK_HEIGHT 	256
#define MAX_CHUNK_BlOCKS 	(CHUNK_WIDTH*CHUNK_WIDTH*MAX_CHUNK_HEIGHT)

typedef struct{
	int x,z;
}ChunkPos;	


typedef struct{
	unsigned int height;
	ChunkPos pos;
	List blocks;
}Chunk;

List world;
#define AM_CHUNKS (world.used/sizeof(Chunk))
#define worldl ((Chunk*)world.l)
#define WORLD_BLOCK_INDEX(i)  ((BlockType*)(worldl[i/MAX_CHUNK_BlOCKS].blocks.l))[i%MAX_CHUNK_BlOCKS]
#define FLOOR_I(f) (unsigned int)floorf(f)
#define POS_TO_BLOCK_INDEX(pos) FLOOR_I(pos.x)%CHUNK_WIDTH*CHUNK_WIDTH + FLOOR_I(pos.y)*CHUNK_WIDTH*CHUNK_WIDTH + FLOOR_I(pos.z)%CHUNK_WIDTH
#define CHUNK_POS_TO_BLOCK(chunk) (Pos){CHUNK_WIDTH*chunk.x,0,CHUNK_WIDTH*chunk.z}
#define INDEX_TO_POS(i)  ((Pos)ADD_VEC(CHUNK_POS_TO_BLOCK(worldl[i/MAX_CHUNK_BlOCKS].pos),((Vec){i%(CHUNK_WIDTH*CHUNK_WIDTH)/CHUNK_WIDTH,i%MAX_CHUNK_BlOCKS/(CHUNK_WIDTH*CHUNK_WIDTH),i%CHUNK_WIDTH})))
#define POS_TO_INDEX(pos,chunk) (getChunk(pos,chunk)*MAX_CHUNK_BlOCKS+POS_TO_BLOCK_INDEX(pos))

bool isCorrectChunk(Pos p, unsigned int chunk_index){
	ChunkPos chunk_pos = worldl[chunk_index].pos;
	return(chunk_pos.x==floorf(p.x/CHUNK_WIDTH) && chunk_pos.z==floorf(p.z/CHUNK_WIDTH));
}

unsigned int getChunk(Pos p, unsigned int i){
	if(i==UINT_MAX)i=AM_CHUNKS/2;
	if(isCorrectChunk(p,i))return i;
	for(unsigned int j=1;i+j<AM_CHUNKS||i>=j;j++){
		if(i+j<AM_CHUNKS && isCorrectChunk(p,i+j))return i+j;
		if(i>=j && isCorrectChunk(p,i-j))return i-j;
	}
	return UINT_MAX;
}


unsigned int is_block(Pos pos, unsigned int chunk_index){
	//int i= (int)(floorf(pos.x)*WORLD_Y*WORLD_Z+floorf(pos.y)*WORLD_Z+floorf(pos.z));
	//int chunk_index = POS_TO_CHUNK_INDEX(pos);
	if(pos.y<0)return(UINT_MAX);
	unsigned int block_index = POS_TO_BLOCK_INDEX(pos);
	chunk_index = getChunk(pos,chunk_index);
	if(chunk_index==UINT_MAX)return UINT_MAX;
	Chunk c = worldl[chunk_index];
	//if(c.type!=RENDERABLE||c.blocks[block_index]==AIR)return UINT_MAX;
	if(c.height<=pos.y || ((BlockType*)c.blocks.l)[block_index]==AIR)return UINT_MAX;
	return(chunk_index * MAX_CHUNK_BlOCKS + block_index);
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


static void draw_block(BlockType type,unsigned int chunk_index,Pos pos,Player* player)
{
	if(type==AIR)return;
	#define SIDE_COLOR(dir) (blockTypes[type][dir]).color
	ALLEGRO_VERTEX faces[6][4]={{
	        //top
	/*   x   y   z   u   v  c  */
	   { SIZE_BLOCK,  SIZE_BLOCK, 0,  0,  0, SIDE_COLOR(Top)},//1
	   { SIZE_BLOCK,  SIZE_BLOCK,  SIZE_BLOCK, 16,  0, SIDE_COLOR(Top)},//2
	   {0,  SIZE_BLOCK, 0,  0, 16, SIDE_COLOR(Top)},//0
	   {0,  SIZE_BLOCK,  SIZE_BLOCK, 16, 16, SIDE_COLOR(Top)},//3
	},{
	        //bottem
	/*   x   y   z   u   v  c  */
	   {0, 0, 0,  0,  0, SIDE_COLOR(Bottem)},//4
	   {0, 0,  SIZE_BLOCK, 16,  0, SIDE_COLOR(Bottem)},//7
	   { SIZE_BLOCK, 0, 0,  0, 16, SIDE_COLOR(Bottem)},//5
	   { SIZE_BLOCK, 0,  SIZE_BLOCK, 16, 16, SIDE_COLOR(Bottem)},//6
	},{
	        //west
	/*   x   y   z   u   v  c  */
	   {0,  SIZE_BLOCK, 0,  0,  0, SIDE_COLOR(West)},//0
	   {0, 0, 0,  0, 16, SIDE_COLOR(West)},//4
	   {0,  SIZE_BLOCK,  SIZE_BLOCK, 16,  0, SIDE_COLOR(West)},//3
	   {0, 0,  SIZE_BLOCK, 16, 16, SIDE_COLOR(West)},//7
	},{
	        //east
	/*   x   y   z   u   v  c  */
	   { SIZE_BLOCK, 0, 0,16,16, SIDE_COLOR(East)},//5
	   { SIZE_BLOCK,  SIZE_BLOCK, 0,16,0, SIDE_COLOR(East)},//1
	   { SIZE_BLOCK, 0,  SIZE_BLOCK,0,16, SIDE_COLOR(East)},//6
	   { SIZE_BLOCK,  SIZE_BLOCK,  SIZE_BLOCK,0,0, SIDE_COLOR(East)},//2
	},{
	        //south
	/*   x   y   z   u   v  c  */
	   {0,  SIZE_BLOCK,  SIZE_BLOCK,  0,  0, SIDE_COLOR(South)},//3
	   { SIZE_BLOCK,  SIZE_BLOCK,  SIZE_BLOCK, 16,  0, SIDE_COLOR(South)},//2
	   {0, 0,  SIZE_BLOCK,  0, 16, SIDE_COLOR(South)},//7
	   { SIZE_BLOCK, 0,  SIZE_BLOCK, 16, 16, SIDE_COLOR(South)},//6
	},{
	        //north
	/*   x   y   z   u   v  c  */
	   {0,  SIZE_BLOCK, 0,  0,  0, SIDE_COLOR(North)},//0
	   { SIZE_BLOCK,  SIZE_BLOCK, 0, 16,  0, SIDE_COLOR(North)},//1
	   {0, 0, 0,  0, 16, SIDE_COLOR(North)},//4
	   { SIZE_BLOCK, 0, 0, 16, 16, SIDE_COLOR(North)},//5
	}};
	
	
	int indices[6] = {
	   0,1,3,
	   0,2,3
	};
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_translate_transform_3d(&t, pos.x*SIZE_BLOCK,
			pos.y*SIZE_BLOCK,
			-pos.z*SIZE_BLOCK-SIZE_BLOCK);
	al_use_transform(&t);
	for(int i=0;i<6;i++){
		//If the side shouldn't be visible there is no point
		//drawing it
		if(lessThan(i,player->pos,pos))continue;

		//only draw the face if there is no face next to it
		Pos adjacent_block_pos = ADD_VEC(pos,dirVectors[i]);
		if(is_block(adjacent_block_pos,chunk_index)==UINT_MAX)
		al_draw_indexed_prim(faces[i], NULL,
				(blockTypes[type][i]).texture,
				indices, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
   	}
}

//#define INDEX_TO_POS(i) (Pos){i/(WORLD_Y*WORLD_Z),(i/WORLD_Z)%WORLD_Y,(i%WORLD_Y)%WORLD_Z}

void draw_map(Player* p)
{
	for(unsigned int i=0;i<AM_CHUNKS;i++){
		Chunk chunk = worldl[i];
		BlockType* blocks = (BlockType*)chunk.blocks.l;
		for(unsigned int j=0;j<chunk.height;j++){
			for(unsigned int k=0;k<CHUNK_WIDTH*CHUNK_WIDTH;k++){
				Pos pos = ADD_VEC(
						(CHUNK_POS_TO_BLOCK(chunk.pos)),
						((Vec){k/CHUNK_WIDTH,j,k%CHUNK_WIDTH})
						);
				draw_block(blocks[j*CHUNK_WIDTH*CHUNK_WIDTH+k],i,pos,p);
			}
		}
	}
}

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

void loadBlocks(){
	bool success = al_change_directory("pics");
	if(!success){
		printf("Could not change directory");
		exit(1);
	}

	for(unsigned int i=0;i<LEN_TEXTURES;i++){
		texturesBMP[i]=al_load_bitmap(texturesStr[i]);
		if(texturesBMP[i]==NULL){
			printf("Could not load %s\n",texturesStr[i]);
			exit(1);
		}
	}
	al_change_directory("..");
	for(unsigned int i=0;i<LEN_BLOCK_TYPES;i++){
		for(unsigned int j=0;j<6;j++){
			Texture t = blockTypesPreProcessing[i][j].texture;
			Color c = blockTypesPreProcessing[i][j].color;
			blockTypes[i][j].texture = texturesBMP[t];
			blockTypes[i][j].color = al_map_rgb(c.r,c.g,c.b);
		}
	}
}


bool isValidPlayerPos(Pos pos, unsigned int chunk_index){
	Pos pos_low=pos;
	pos_low.y--;
	return(is_block(pos_low,chunk_index)==UINT_MAX//Checks if the lower part of the player's body is
					 //in a valid position
			&& is_block(pos,chunk_index)==UINT_MAX);
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
	unsigned int new_chunk_index = getChunk(new_pos, p->chunk_index);

	if(!checkValid ||  isValidPlayerPos(new_pos_extra,new_chunk_index)){
		p->pos=new_pos;
		p->chunk_index=new_chunk_index;
	}
}


Direction hitDirectionBlock(Pos prev_pos, Pos block_pos){
	if(prev_pos.y>block_pos.y+1)return Top;
	else if(prev_pos.y<block_pos.y)return Bottem;
	else if(prev_pos.x<block_pos.x)return West;
	else if(prev_pos.x>block_pos.x+1)return East;
	else if(prev_pos.z<block_pos.z)return South;
	else if(prev_pos.z>block_pos.z+1)return North;
	else return NoDirection;
}


unsigned int ray_block(Direction* dir,Vec v,Pos pos,unsigned int chunk_index){
	for(unsigned int i=0;i<100;i++){
		Pos prev_pos = pos;
		pos =(Pos)ADD_VEC(v,pos);
		unsigned int ret;
		if((ret=is_block(pos,chunk_index))!=UINT_MAX){
			if(dir)*dir=hitDirectionBlock(prev_pos,
					INDEX_TO_POS(ret));
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

void setBlock(int index, int chunk_index,BlockType type){
	Pos pos = INDEX_TO_POS(index);
	if(pos.y>MAX_CHUNK_HEIGHT ||  pos.y<0)return;
	chunk_index =getChunk(pos,chunk_index);
	Chunk* chunk = &(worldl[chunk_index]);
	if(pos.y>=chunk->height){
		//increase the height of the chunk
		int increaseWith = pos.y-chunk->height+10;
		BlockType justAir[increaseWith*CHUNK_WIDTH*CHUNK_WIDTH];
		memset(justAir,AIR,sizeof(BlockType)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH);
		list_concat(&(chunk->blocks),sizeof(BlockType)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH,justAir);
		chunk->height+=increaseWith;
	}
	WORLD_BLOCK_INDEX(index) = type;
}

//Gives the distance of x to [a,b] where a<=b
float distanceInterval(float x,float a,float b){
	if(x<=a)return(a-x);
	else if(b<=x)return(x-b);
	else return(0);
}

Chunk renderChunk(ChunkPos pos){
	Chunk ret = {10,pos,list_init(10*CHUNK_WIDTH*CHUNK_WIDTH*sizeof(BlockType))};
	memset(ret.blocks.l,AIR,ret.blocks.used);
	for(int i=0;i<4*CHUNK_WIDTH*CHUNK_WIDTH;i++){
		((BlockType*)(ret.blocks.l))[i]=STONE_BLOCK;
	}
	for(int i=4*CHUNK_WIDTH*CHUNK_WIDTH;i<5*CHUNK_WIDTH*CHUNK_WIDTH;i++){
			((BlockType*)(ret.blocks.l))[i]=GRASS_BLOCK;
	}
	return ret;
}

void loadChunks(Player p){
	for(int i=-WORLD_CHUNKS;i<=WORLD_CHUNKS;i++){
	for(int j=-WORLD_CHUNKS;j<=WORLD_CHUNKS;j++){
		if(getChunk(ADD_VEC(((Pos){CHUNK_WIDTH*i,0,CHUNK_WIDTH*j}),(p.pos)),p.chunk_index)
				==UINT_MAX){
			list_append(&world,sizeof(Chunk));
			worldl[AM_CHUNKS-1]=
				renderChunk((ChunkPos){i+(int)p.pos.x/CHUNK_WIDTH,j+(int)p.pos.z/CHUNK_WIDTH});
		}
	}
	}
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
		loadChunks(player);

		if(!flying){
		Pos new_pos = player.pos;
		new_pos.y+=player.vert_speed/FRAMERATE;

		Pos foot;
		foot=new_pos;
		foot.y-=1.8;
		unsigned int b= is_block(foot,player.chunk_index);
		if(b==UINT_MAX){
			Pos prev_block_pos;
			if(!player.falling)
				prev_block_pos = INDEX_TO_POS(standing_on);
			if(	standing_on==UINT_MAX ||
				WORLD_BLOCK_INDEX(standing_on)==AIR
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
					case 2://place block
						//Pos new_pos=((Block*)block_list.l)
						//	[block_index].pos;
						if(dir==NoDirection)break;
						Pos block = INDEX_TO_POS(block_index);
						int chunk_index = getChunk((ADD_VEC(INDEX_TO_POS(block_index), dirVectors[dir])),block_index/MAX_CHUNK_BlOCKS);
						ChunkPos p =worldl[chunk_index].pos;
						block_index = POS_TO_INDEX((ADD_VEC(INDEX_TO_POS(block_index), dirVectors[dir])), block_index/MAX_CHUNK_BlOCKS);
						//TODO: Check if the new block doesn't go out of
						setBlock(block_index,chunk_index,block_selection);
						break;
					case 1://break block
					        //WORLD_BLOCK_INDEX(block_index)=AIR;
						setBlock(block_index,block_index/MAX_CHUNK_BlOCKS,AIR);
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
			draw_map(&player);
			
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
