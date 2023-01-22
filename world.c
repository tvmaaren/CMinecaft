#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdlib.h>

#include "list.h"
#include "types.h"
#include "block.h"
#include "world.h"
#include "noise.c"
#define AM_CHUNKS (world->used/sizeof(Chunk))


bool isCorrectChunk(List* world, ChunkPos p, unsigned int chunk_index){
	ChunkPos chunk_pos = worldl(world)[chunk_index].pos;
	return(chunk_pos.x==p.x && chunk_pos.z==p.z);
}

unsigned int getChunk(List* world,ChunkPos p, unsigned int i){
	if(i==UINT_MAX)i=AM_CHUNKS/2;
	if(isCorrectChunk(world,p,i))return i;
	for(unsigned int j=1;i+j<AM_CHUNKS||i>=j;j++){
		if(i+j<AM_CHUNKS && isCorrectChunk(world,p,i+j))return i+j;
		if(i>=j && isCorrectChunk(world,p,i-j))return i-j;
	}
	return UINT_MAX;
}

unsigned int is_block(List* world, Pos pos, unsigned int chunk_index){
	if(pos.y<0)return(UINT_MAX);
	unsigned int block_index = POS_TO_BLOCK_INDEX(pos);
	chunk_index = getChunk(world,POS_TO_CHUNK_POS(pos),chunk_index);
	if(chunk_index==UINT_MAX)return UINT_MAX;
	Chunk c = worldl(world)[chunk_index];
	if(c.height<=pos.y || ((BlockTypeEnum*)c.blocks.l)[block_index]==AIR)return UINT_MAX;
	return(chunk_index * MAX_CHUNK_BlOCKS + block_index);
}

void reCreateWorldMesh(List* world, List* worldMesh,Player* p){
	for(int i=0;i<(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1);i++){
		list_free(worldMesh[i]);
		worldMesh[i]=list_init(0);
	}
	createWorldMesh(world, worldMesh, p);
}

void createChunkMesh(List* world,List* chunkMesh,Player* p,int i){
	ChunkPos playerChunkPos = POS_TO_CHUNK_POS(p->pos);
	List transparentFaces = list_init(0);

	ChunkPos chunkPos = {	playerChunkPos.x+(-WORLD_CHUNKS+i/(2*WORLD_CHUNKS+1)),
				playerChunkPos.z+(-WORLD_CHUNKS+i%(2*WORLD_CHUNKS+1))};
	int chunkIndex = getChunk(world,chunkPos, p->chunk_index);
	Chunk chunk = worldl(world)[chunkIndex];
	BlockTypeEnum* blocks = (BlockTypeEnum*)chunk.blocks.l;
	for(unsigned int h=0;h<chunk.height;h++){
		for(unsigned int w=0;w<CHUNK_WIDTH*CHUNK_WIDTH;w++){
			Pos pos = ADD_VEC(CHUNK_POS_TO_BLOCK(chunkPos),
					((Vec){w/CHUNK_WIDTH,h,w%CHUNK_WIDTH}));
			createBlockMesh(world,chunkMesh,&transparentFaces,
					blocks[h*CHUNK_WIDTH*CHUNK_WIDTH+w],chunkIndex,pos);
		}
	}
	list_concat(chunkMesh,transparentFaces.used,transparentFaces.l);
	list_free(transparentFaces);
}
void createWorldMesh(List* world,List* worldMesh,Player* p){
	for(int i=0;i<(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1);i++){
		createChunkMesh(world,&(worldMesh[i]),p,i);
	}
}

void drawChunk(List* chunkMesh, Player*p){
	for(int j=0;j<(chunkMesh->used)/sizeof(Side);j++){
		draw_face(((Side*)chunkMesh->l)[j],p);
	}
}

void draw_world(List* worldMesh,Player *p){
	//first draw the outer chunks
	for(int i=WORLD_CHUNKS;i>0;i--){
		int NW = (2*WORLD_CHUNKS+1)*(WORLD_CHUNKS-i)+WORLD_CHUNKS-i;
		int NE = NW+2*i;
		int SW = NW+2*i*(2*WORLD_CHUNKS+1);
		int SE = SW+2*i;

		drawChunk(&(worldMesh[NW]),p);
		drawChunk(&(worldMesh[NE]),p);
		drawChunk(&(worldMesh[SW]),p);
		drawChunk(&(worldMesh[SE]),p);

		for(int k=1;k<i;k++){
			drawChunk(&(worldMesh[NW+k]),p);
			drawChunk(&(worldMesh[NW+(2*WORLD_CHUNKS+1)*k]),p);

			drawChunk(&(worldMesh[NE-k]),p);
			drawChunk(&(worldMesh[NE+(2*WORLD_CHUNKS+1)*k]),p);

			drawChunk(&(worldMesh[SW+k]),p);
			drawChunk(&(worldMesh[SW-(2*WORLD_CHUNKS+1)*k]),p);

			drawChunk(&(worldMesh[SE-k]),p);
			drawChunk(&(worldMesh[SE-(2*WORLD_CHUNKS+1)*k]),p);
		}
		drawChunk(&(worldMesh[NW+i]),p);
		drawChunk(&(worldMesh[NE+i*(2*WORLD_CHUNKS+1)]),p);
		drawChunk(&(worldMesh[SW-i*(2*WORLD_CHUNKS+1)]),p);
		drawChunk(&(worldMesh[SE-i]),p);
	}
	//Draw the chunk where the player is located
	drawChunk(&(worldMesh[WORLD_CHUNKS*(2*WORLD_CHUNKS+1)+WORLD_CHUNKS]),p);
}


void setBlock(List* world, int index, int chunk_index,BlockTypeEnum type){
	Pos pos = INDEX_TO_POS(world,index);
	if(pos.y>MAX_CHUNK_HEIGHT ||  pos.y<0)return;
	chunk_index =getChunk(world,POS_TO_CHUNK_POS(pos),chunk_index);
	Chunk* chunk = &(worldl(world)[chunk_index]);
	if(pos.y>=chunk->height){
		//increase the height of the chunk
		int increaseWith = pos.y-chunk->height+10;
		BlockTypeEnum justAir[increaseWith*CHUNK_WIDTH*CHUNK_WIDTH];
		memset(justAir,AIR,sizeof(BlockTypeEnum)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH);
		list_concat(&(chunk->blocks),sizeof(BlockTypeEnum)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH,justAir);
		chunk->height+=increaseWith;
	}
	WORLD_BLOCK_INDEX(world,index) = type;
}

float levelfunc(int x,int z){
	return(20+perlin(((float)x)/16,((float)z)/16)*5+perlin(((float)x)/50,((float)z)/50)*30);
}
	
Chunk renderChunk(ChunkPos pos){
	int r = random()%(CHUNK_WIDTH*CHUNK_WIDTH);
	Chunk ret = {50,pos,list_init(50*CHUNK_WIDTH*CHUNK_WIDTH*sizeof(BlockTypeEnum))};
	memset(ret.blocks.l,AIR,ret.blocks.used);
	for(int i=0;i<CHUNK_WIDTH*CHUNK_WIDTH;i++){
		int j;
		for(j =i;j/(CHUNK_WIDTH*CHUNK_WIDTH)<levelfunc(CHUNK_WIDTH*pos.x+i/CHUNK_WIDTH,CHUNK_WIDTH*pos.z+i%CHUNK_WIDTH);j+=(CHUNK_WIDTH*CHUNK_WIDTH)){
			((BlockTypeEnum*)(ret.blocks.l))[j]=STONE_BLOCK;
		}
		for(int k=0;k<3;k++){
			((BlockTypeEnum*)(ret.blocks.l))[j]=DIRT_BLOCK;
			j+=(CHUNK_WIDTH*CHUNK_WIDTH);
		}
		((BlockTypeEnum*)(ret.blocks.l))[j]=GRASS_BLOCK;
		if(i==r){
			for(int k=0;k<3;k++){
				j+=(CHUNK_WIDTH*CHUNK_WIDTH);
				((BlockTypeEnum*)(ret.blocks.l))[j]=OAK_LOG_BLOCK;
			}
			((BlockTypeEnum*)(ret.blocks.l))[j-1]=OAK_LEAVES_BLOCK;
			((BlockTypeEnum*)(ret.blocks.l))[j+1]=OAK_LEAVES_BLOCK;
			((BlockTypeEnum*)(ret.blocks.l))[j+CHUNK_WIDTH]=OAK_LEAVES_BLOCK;
			((BlockTypeEnum*)(ret.blocks.l))[j-CHUNK_WIDTH]=OAK_LEAVES_BLOCK;
			((BlockTypeEnum*)(ret.blocks.l))[j+CHUNK_WIDTH*CHUNK_WIDTH]=OAK_LEAVES_BLOCK;
		}
	}
	return ret;
}


void loadChunks(List* world,Player p){
	ChunkPos playerChunkPos = POS_TO_CHUNK_POS(p.pos);
	for(int i=0;i<(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1);i++){
		ChunkPos chunkPos = {	playerChunkPos.x+(-WORLD_CHUNKS+i/(2*WORLD_CHUNKS+1)),
				  	playerChunkPos.z+(-WORLD_CHUNKS+i%(2*WORLD_CHUNKS+1))};
		int chunkIndex = getChunk(world,chunkPos, p.chunk_index);
		if(chunkIndex==UINT_MAX){
			list_append(world,sizeof(Chunk));
			worldl(world)[AM_CHUNKS-1]=renderChunk(chunkPos);
		}
	}
}
