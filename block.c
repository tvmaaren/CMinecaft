#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <math.h>

#define ICON_WIDTH_HEIGHT 100

#include "list.h"
#include "types.h"
#include "chunk.h"
#include "block.h"
#include "world.h"

Vec dirVectors[6]={{0,1,0},{0,-1,0},{-1,0,0},{1,0,0},{0,0,-1},{0,0,1}};
void set_other_transform(ALLEGRO_BITMAP* bmp)
{
	ALLEGRO_TRANSFORM p;
	float aspect_ratio = (float)al_get_bitmap_height(bmp) / al_get_bitmap_width(bmp);
	al_set_target_bitmap(bmp);
	al_identity_transform(&p);
	al_translate_transform_3d(&p,-4,-4,0);
	al_rotate_transform_3d(&p,0,1,0,-ALLEGRO_PI/4);
	al_translate_transform_3d(&p,0,-10,-20);
	al_perspective_transform(&p, -0.25, 0.0*aspect_ratio, 1, 0.5, -0.85*aspect_ratio, 1000);
	al_use_projection_transform(&p);
}
void createIcon(ALLEGRO_BITMAP** bmp, BlockTypeEnum type,int width,int height){
	*bmp = al_create_bitmap(width,height);
	al_set_target_bitmap(*bmp);
	al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
	al_clear_to_color(al_map_rgba_f(0,0,0,0));
	al_clear_depth_buffer(1000);
	set_other_transform(*bmp);

	draw_face((Side){type,(IPos){0,0,0},South},NULL);
	draw_face((Side){type,(IPos){0,0,0},East},NULL);
	draw_face((Side){type,(IPos){0,0,0},Top},NULL);
}


char* texturesStr[] = {"dirt.png", "grass_block_side.png",
	"grass_block_top.png", "cobblestone.png", "oak_planks.png",
	"oak_log.png", "oak_log_top.png","stone.png","oak_leaves.png",
	"glass.png","water_still.png"};
#define LEN_TEXTURES  sizeof(texturesStr)/sizeof(char*)
ALLEGRO_BITMAP* texturesBMP[LEN_TEXTURES];

#define WHITE {255,255,255}
#define GREEN {72,181,24}
#define WATER_BLUE {0x3F,0x76,0xE4}
BlockTypePreProcessing blockTypesPreProcessing[] =
{
	{{/*grass_block*/{GRASS_BLOCK_TOP, {124,  189,  1}}, {DIRT, WHITE},
	{GRASS_BLOCK_SIDE, WHITE}, {GRASS_BLOCK_SIDE, WHITE},
	{GRASS_BLOCK_SIDE, WHITE}, {GRASS_BLOCK_SIDE, WHITE}},false },

	{{/*dirt_block*/{DIRT, WHITE}, {DIRT, WHITE}, {DIRT, WHITE}, {DIRT, WHITE},
	{DIRT, WHITE}, {DIRT, WHITE}},false}, 

	{{/*cobblestone*/{COBBLESTONE, WHITE}, {COBBLESTONE, WHITE},
		{COBBLESTONE, WHITE}, {COBBLESTONE, WHITE}, {COBBLESTONE, WHITE},
		{COBBLESTONE, WHITE}},false}, 
	{{/*oak_planks*/{OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE},
	{OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE}, {OAK_PLANKS, WHITE}},false}, 

	{{/*oak_log*/{OAK_LOG_TOP, WHITE}, {OAK_LOG_TOP, WHITE}, {OAK_LOG, WHITE},
	{OAK_LOG, WHITE}, {OAK_LOG, WHITE}, {OAK_LOG, WHITE}},false}, 

	{{/*stone_block*/{STONE,WHITE}, {STONE,WHITE}, {STONE,WHITE}, {STONE,WHITE},
	{STONE, WHITE}, {STONE, WHITE}},false},
	
	{{/*oak_leaves*/{OAK_LEAVES,GREEN}, {OAK_LEAVES,GREEN}, {OAK_LEAVES,GREEN}, {OAK_LEAVES,GREEN},
	{OAK_LEAVES, GREEN}, {OAK_LEAVES, GREEN}},true},
	
	{{/*glass_block*/{GLASS,WHITE}, {GLASS,WHITE}, {GLASS,WHITE}, {GLASS,WHITE},
	{GLASS, WHITE}, {GLASS, WHITE}},true},
	
	{{/*water_block*/{WATER,WATER_BLUE}, {WATER,WATER_BLUE}, {WATER,WATER_BLUE}, {WATER,WATER_BLUE},
	{WATER, WATER_BLUE}, {WATER, WATER_BLUE}},true}
};
#define LEN_BLOCK_TYPES sizeof(blockTypesPreProcessing)/sizeof(BlockTypePreProcessing)
BlockType blockTypes[LEN_BLOCK_TYPES];

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

Pos adjacentPos(Pos p,  Direction d){
	return(ADD_VEC(p,dirVectors[d]));
}



const ALLEGRO_VERTEX block_faces[6][4]={{
	        //top
	/*   x   y   z   u   v  c  */
	   { SIZE_BLOCK,  SIZE_BLOCK, 0,  0,  0,0},//1
	   { SIZE_BLOCK,  SIZE_BLOCK,  SIZE_BLOCK, 16,  0,0},//2
	   {0,  SIZE_BLOCK, 0,  0, 16,0},//0
	   {0,  SIZE_BLOCK,  SIZE_BLOCK, 16, 16,0},//3
	},{
	        //bottem
	/*   x   y   z   u   v  c  */
	   {0, 0, 0,  0,  0,0},//4
	   {0, 0,  SIZE_BLOCK, 16,  0,0},//7
	   { SIZE_BLOCK, 0, 0,  0, 16,0},//5
	   { SIZE_BLOCK, 0,  SIZE_BLOCK, 16, 16,0},//6
	},{
	        //west
	/*   x   y   z   u   v  c  */
	   {0,  SIZE_BLOCK, 0,  0,  0,0},//0
	   {0, 0, 0,  0, 16,0},//4
	   {0,  SIZE_BLOCK,  SIZE_BLOCK, 16,  0,0},//3
	   {0, 0,  SIZE_BLOCK, 16, 16,0},//7
	},{
	        //east
	/*   x   y   z   u   v  c  */
	   { SIZE_BLOCK, 0, 0,16,16,0},//5
	   { SIZE_BLOCK,  SIZE_BLOCK, 0,16,0,0},//1
	   { SIZE_BLOCK, 0,  SIZE_BLOCK,0,16,0},//6
	   { SIZE_BLOCK,  SIZE_BLOCK,  SIZE_BLOCK,0,0,0},//2
	},{
	        //south
	/*   x   y   z   u   v  c  */
	   {0,  SIZE_BLOCK,  SIZE_BLOCK,  0,  0,0},//3
	   { SIZE_BLOCK,  SIZE_BLOCK,  SIZE_BLOCK, 16,  0,0},//2
	   {0, 0,  SIZE_BLOCK,  0, 16,0},//7
	   { SIZE_BLOCK, 0,  SIZE_BLOCK, 16, 16,0},//6
	},{
	        //north
	/*   x   y   z   u   v  c  */
	   {0,  SIZE_BLOCK, 0,  0,  0,0},//0
	   { SIZE_BLOCK,  SIZE_BLOCK, 0, 16,  0,0},//1
	   {0, 0, 0,  0, 16,0},//4
	   { SIZE_BLOCK, 0, 0, 16, 16,0},//5
	}};
void draw_face(Side face, Player *p){
	if(p!=NULL && lessThan(face.side, p->pos, CAST_TO_POS(face.pos)))return;
	float x=face.pos.x*SIZE_BLOCK;
	float y=face.pos.y*SIZE_BLOCK;
	float z=-face.pos.z*SIZE_BLOCK-SIZE_BLOCK;
	#define f block_faces[face.side]
	#define SIDE_COLOR(dir) (blockTypes[face.type].sides[dir]).color
	ALLEGRO_VERTEX vertices[4]={
	/*	x 	y 	z 	u 	v 	c*/
		{f[0].x+x,f[0].y+y,	f[0].z+z,	f[0].u,	f[0].v,	SIDE_COLOR(face.side)},
		{f[1].x+x,f[1].y+y,	f[1].z+z,	f[1].u,	f[1].v,	SIDE_COLOR(face.side)},
		{f[2].x+x,f[2].y+y,	f[2].z+z,	f[2].u,	f[2].v,	SIDE_COLOR(face.side)},
		{f[3].x+x,f[3].y+y,	f[3].z+z,	f[3].u,	f[3].v,	SIDE_COLOR(face.side)}
	};
	int indices[6] = {
	   0,1,3,
	   0,2,3
	};
	al_draw_indexed_prim(vertices,NULL,
			blockTypes[face.type].sides[face.side].texture,
			indices,6, ALLEGRO_PRIM_TRIANGLE_LIST);
}

Direction isOnChunkEdge(IPos pos, Direction dir){
	switch(dir){
		case(West):
			return pos.x%CHUNK_WIDTH==0;
		case(East):	
			return pos.x%CHUNK_WIDTH==CHUNK_WIDTH-1;
		case(South):
			return pos.z%CHUNK_WIDTH==0;
		case(North):
			return pos.z%CHUNK_WIDTH==CHUNK_WIDTH-1;
		default: 
			return false;
	}
}

void createBlockSideMesh(List* world,Chunk* chunk,List* transparentBlocks,
		BlockTypeEnum type,unsigned int chunk_index,IPos pos, Direction i){
	if(type==AIR)return;
	//only draw the face if there is no face next to it
	List* chunkMesh= &(chunk->mesh);
	Pos adjacent_block_pos = ADD_VEC(pos,dirVectors[i]);

	//The player should never come at the bottom of the world
	//so there is no point drawing the face
	if(adjacent_block_pos.y<0)return;

	int chunk_adjacent = getChunk(world,POS_TO_CHUNK_POS(adjacent_block_pos),chunk_index);
	BlockTypeEnum type_adjacent;
	if(chunk_adjacent==UINT_MAX){
		type_adjacent=AIR;
	}else{
		unsigned int chunk = getChunk(world,(POS_TO_CHUNK_POS(adjacent_block_pos)),chunk_index);
		unsigned int block_index = POS_TO_BLOCK_INDEX(adjacent_block_pos);
		unsigned int index = chunk*MAX_CHUNK_BlOCKS+block_index;
		//int index = POS_TO_INDEX(world,adjacent_block_pos,chunk_index);
//#define POS_TO_INDEX(world,pos,chunk) (getChunk(world,(POS_TO_CHUNK_POS(pos)),chunk)*MAX_CHUNK_BlOCKS+POS_TO_BLOCK_INDEX(pos))
		type_adjacent=WORLD_BLOCK_INDEX(world,index);
		if(worldl(world)[chunk_adjacent].height<=adjacent_block_pos.y || adjacent_block_pos.y<0)type_adjacent =AIR;
	}
	if(	chunk_adjacent == UINT_MAX
		||(type_adjacent !=AIR &&!blockTypes[type].transparent &&  blockTypes[type_adjacent].transparent)
			||is_block(world,adjacent_block_pos,chunk_index)==UINT_MAX){
		bool test = blockTypes[type].transparent;
		if(blockTypes[type].transparent){
			list_append(transparentBlocks,sizeof(Side));
			((Side*)transparentBlocks->l)[transparentBlocks->used/sizeof(Side)-1]=(Side){type,pos,i};
		}else{
			bool a = isOnChunkEdge(pos, i);
			bool b = i>=2&&a;
			if(b){
				List* sideMesh = &(chunk->sideMeshes[i-2]);
				list_append(sideMesh,sizeof(Side));
				((Side*)sideMesh->l)[sideMesh->used/sizeof(Side)-1]=(Side){type,pos,i};
			}else{
				list_append(chunkMesh,sizeof(Side));
				((Side*)chunkMesh->l)[chunkMesh->used/sizeof(Side)-1]=(Side){type,pos,i};
			}
		}
	}
}

void createBlockMesh(List* world,Chunk* chunk,List* transparentBlocks, BlockTypeEnum type,unsigned int chunk_index,IPos pos)
{
	if(type==AIR)return;
	
	for(int i=0;i<6;i++){
		if(type==1028098){
		}
		createBlockSideMesh(world,chunk,transparentBlocks,type, chunk_index, pos ,i);
	}
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
		blockTypes[i].transparent = blockTypesPreProcessing[i].transparent;
		for(unsigned int j=0;j<6;j++){
			Texture t = blockTypesPreProcessing[i].sides[j].texture;
			Color c = blockTypesPreProcessing[i].sides[j].color;
			blockTypes[i].sides[j].texture = texturesBMP[t];
			blockTypes[i].sides[j].color = al_map_rgb(c.r,c.g,c.b);
		}
		createIcon(&(blockTypes[i].bmp), i,ICON_WIDTH_HEIGHT,ICON_WIDTH_HEIGHT);
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


void drawHotbar(float x,float y,float width,float height,int select,BlockTypeEnum* hotbar){
	float selectX1 = x+height*select;
	float selectX2 = selectX1+height;
	al_draw_rectangle(selectX1,y,selectX2,y+height,
			al_map_rgb_f(1,0,0),2);
	float x1;
	float x2=x;
	for(int j=0;j<9;j++){
		x1 = x2;
		x2=x1+height;
		if(select!=j)
		al_draw_rectangle(x1,y,x2,y+height,
				al_map_rgb_f(1,1,1),2);
		al_draw_scaled_bitmap(blockTypes[hotbar[j]].bmp,
				0,0,ICON_WIDTH_HEIGHT,ICON_WIDTH_HEIGHT,
				x1,y,height,height,0);
	}
}
// vim: cc=100 
