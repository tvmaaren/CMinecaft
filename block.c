#include <allegro5/allegro_primitives.h>
#include <stdio.h>

#include "list.h"
#include "types.h"
#include "block.h"
#include "world.h"

char* texturesStr[] = {"dirt.png", "grass_block_side.png",
	"grass_block_top.png", "cobblestone.png", "oak_planks.png",
	"oak_log.png", "oak_log_top.png","stone.png"};
#define LEN_TEXTURES  sizeof(texturesStr)/sizeof(char*)
ALLEGRO_BITMAP* texturesBMP[LEN_TEXTURES];

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

void draw_block(List* world,BlockType type,unsigned int chunk_index,Pos pos,Player* player)
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
		if(is_block(world,adjacent_block_pos,chunk_index)==UINT_MAX)
		al_draw_indexed_prim(faces[i], NULL,
				(blockTypes[type][i]).texture,
				indices, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
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
		for(unsigned int j=0;j<6;j++){
			Texture t = blockTypesPreProcessing[i][j].texture;
			Color c = blockTypesPreProcessing[i][j].color;
			blockTypes[i][j].texture = texturesBMP[t];
			blockTypes[i][j].color = al_map_rgb(c.r,c.g,c.b);
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
