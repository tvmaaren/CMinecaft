//#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
//#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

#include "list.h"
#include "types.h"
#include "block.h"
#include "world.h"
#define AM_CHUNKS (world->used/sizeof(Chunk))

bool isCorrectChunk(List* world, Pos p, unsigned int chunk_index){
	ChunkPos chunk_pos = worldl(world)[chunk_index].pos;
	return(chunk_pos.x==floorf(p.x/CHUNK_WIDTH) && chunk_pos.z==floorf(p.z/CHUNK_WIDTH));
}

unsigned int getChunk(List* world,Pos p, unsigned int i){
	if(i==UINT_MAX)i=AM_CHUNKS/2;
	if(isCorrectChunk(world,p,i))return i;
	for(unsigned int j=1;i+j<AM_CHUNKS||i>=j;j++){
		if(i+j<AM_CHUNKS && isCorrectChunk(world,p,i+j))return i+j;
		if(i>=j && isCorrectChunk(world,p,i-j))return i-j;
	}
	return UINT_MAX;
}

unsigned int is_block(List* world, Pos pos, unsigned int chunk_index){
	//int i= (int)(floorf(pos.x)*WORLD_Y*WORLD_Z+floorf(pos.y)*WORLD_Z+floorf(pos.z));
	//int chunk_index = POS_TO_CHUNK_INDEX(pos);
	if(pos.y<0)return(UINT_MAX);
	unsigned int block_index = POS_TO_BLOCK_INDEX(pos);
	chunk_index = getChunk(world,pos,chunk_index);
	if(chunk_index==UINT_MAX)return UINT_MAX;
	Chunk c = worldl(world)[chunk_index];
	//if(c.type!=RENDERABLE||c.blocks[block_index]==AIR)return UINT_MAX;
	if(c.height<=pos.y || ((BlockType*)c.blocks.l)[block_index]==AIR)return UINT_MAX;
	return(chunk_index * MAX_CHUNK_BlOCKS + block_index);
}

void draw_world(List* world,Player* p)
{
	for(unsigned int i=0;i<AM_CHUNKS;i++){
		Chunk chunk = worldl(world)[i];
		BlockType* blocks = (BlockType*)chunk.blocks.l;
		for(unsigned int j=0;j<chunk.height;j++){
			for(unsigned int k=0;k<CHUNK_WIDTH*CHUNK_WIDTH;k++){
				Pos pos = ADD_VEC(
						(CHUNK_POS_TO_BLOCK(chunk.pos)),
						((Vec){k/CHUNK_WIDTH,j,k%CHUNK_WIDTH})
						);
				draw_block(world,blocks[j*CHUNK_WIDTH*CHUNK_WIDTH+k],i,pos,p);
			}
		}
	}
}

void setBlock(List* world, int index, int chunk_index,BlockType type){
	Pos pos = INDEX_TO_POS(world,index);
	if(pos.y>MAX_CHUNK_HEIGHT ||  pos.y<0)return;
	chunk_index =getChunk(world,pos,chunk_index);
	Chunk* chunk = &(worldl(world)[chunk_index]);
	if(pos.y>=chunk->height){
		//increase the height of the chunk
		int increaseWith = pos.y-chunk->height+10;
		BlockType justAir[increaseWith*CHUNK_WIDTH*CHUNK_WIDTH];
		memset(justAir,AIR,sizeof(BlockType)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH);
		list_concat(&(chunk->blocks),sizeof(BlockType)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH,justAir);
		chunk->height+=increaseWith;
	}
	WORLD_BLOCK_INDEX(world,index) = type;
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

void loadChunks(List* world,Player p){
	for(int i=-WORLD_CHUNKS;i<=WORLD_CHUNKS;i++){
	for(int j=-WORLD_CHUNKS;j<=WORLD_CHUNKS;j++){
		if(getChunk(world,ADD_VEC(((Pos){CHUNK_WIDTH*i,0,CHUNK_WIDTH*j}),(p.pos)),p.chunk_index)
				==UINT_MAX){
			list_append(world,sizeof(Chunk));
			worldl(world)[AM_CHUNKS-1]=
				renderChunk((ChunkPos){i+(int)p.pos.x/CHUNK_WIDTH,j+(int)p.pos.z/CHUNK_WIDTH});
		}
	}
	}
}
