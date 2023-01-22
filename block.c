#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <math.h>

#include "list.h"
#include "types.h"
#include "block.h"
#include "world.h"


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
Vec dirVectors[6]={{0,1,0},{0,-1,0},{-1,0,0},{1,0,0},{0,0,-1},{0,0,1}};

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
	if(lessThan(face.side, p->pos, face.pos))return;
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

void createBlockMesh(List* world,List* chunkMesh,List* transparentBlocks, BlockTypeEnum type,unsigned int chunk_index,Pos pos)
{
	if(type==AIR)return;
	
	for(int i=0;i<6;i++){
		//only draw the face if there is no face next to it
		Pos adjacent_block_pos = ADD_VEC(pos,dirVectors[i]);
		int chunk_adjacent = getChunk(world,POS_TO_CHUNK_POS(adjacent_block_pos),chunk_index);
		if(chunk_adjacent==UINT_MAX)continue;
		BlockTypeEnum type_adjacent;
		int index = POS_TO_INDEX(world,adjacent_block_pos,chunk_index);
		type_adjacent=WORLD_BLOCK_INDEX(world,index);
		if(worldl(world)[chunk_adjacent].height<=adjacent_block_pos.y || adjacent_block_pos.y<0)type_adjacent =AIR;
		if(	chunk_adjacent == UINT_MAX
			||(!blockTypes[type].transparent && type_adjacent !=AIR && blockTypes[type_adjacent].transparent)
				||is_block(world,adjacent_block_pos,chunk_index)==UINT_MAX){
			if(blockTypes[type].transparent){
				list_append(transparentBlocks,sizeof(Side));
				((Side*)transparentBlocks->l)[transparentBlocks->used/sizeof(Side)-1]=(Side){type,pos,i};
			}else{
				list_append(chunkMesh,sizeof(Side));
				((Side*)chunkMesh->l)[chunkMesh->used/sizeof(Side)-1]=(Side){type,pos,i};
			}
		}
   	}
}
void draw_block(List* world,List* transparentBlocks, BlockTypeEnum type,unsigned int chunk_index,Pos pos,Player* player)
{
	if(type==AIR)return;
	
	
	for(int i=0;i<6;i++){
		//If the side shouldn't be visible there is no point
		//drawing it
		if(lessThan(i,player->pos,pos))continue;

		//only draw the face if there is no face next to it
		Pos adjacent_block_pos = ADD_VEC(pos,dirVectors[i]);
		int chunk_adjacent = getChunk(world,POS_TO_CHUNK_POS(adjacent_block_pos),chunk_index);
		int index = POS_TO_INDEX(world,adjacent_block_pos,chunk_index);
		BlockTypeEnum type_adjacent =WORLD_BLOCK_INDEX(world,index);
		if(	chunk_adjacent |= UINT_MAX
			||(blockTypes[type].transparent && type_adjacent !=AIR && blockTypes[type_adjacent].transparent)
				||is_block(world,adjacent_block_pos,chunk_index)==UINT_MAX)
			if(blockTypes[type].transparent){
				list_append(transparentBlocks,sizeof(Side));
				((Side*)transparentBlocks->l)[transparentBlocks->used/sizeof(Side)-1]=(Side){type,pos,i};
			}else
				draw_face((Side){type,pos,i},player);
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
