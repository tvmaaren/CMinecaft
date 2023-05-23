#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "list.h"
#include "types.h"
#include "chunk.h"
#include "block.h"
#include "world.h"
#include "noise.c"
#define AM_CHUNKS (world->used/sizeof(Chunk))
#define WATER_LEVEL 20

extern Vec dirVectors[6];//={{0,1,0},{0,-1,0},{-1,0,0},{1,0,0},{0,0,-1},{0,0,1}};

void saveWorld(FILE* restrict stream, List* world){
	fwrite(world,sizeof(List),1,stream);
	fwrite(world->l,world->used,1,stream);
	for(int i = 0; i<AM_CHUNKS;i++){
		List* pBlocks = &worldl(world)[i].blocks;
		fwrite(pBlocks->l,pBlocks->used,1,stream);
		List* mesh = &worldl(world)[i].mesh;
		fwrite(mesh->l,mesh->used,1,stream);
	}
}

void loadWorld(FILE* restrict stream, List* world){
	fread(world, sizeof(List), 1, stream);
	world->l=malloc(world->available);
	fread(world->l, world->used, 1, stream);
	for(int i=0; i<AM_CHUNKS; i++){
		List* pBlocks = &worldl(world)[i].blocks;
		pBlocks->l = malloc(pBlocks->available);
		fread(pBlocks->l,pBlocks->used,1,stream);
		List* mesh = &worldl(world)[i].mesh;
		mesh->l = malloc(mesh->available);
		fread(mesh->l,mesh->used,1,stream);

	}
}

bool isCorrectChunk(List* world, ChunkPos p, unsigned int chunk_index){
	ChunkPos chunk_pos = worldl(world)[chunk_index].pos;
	return(chunk_pos.x==p.x && chunk_pos.z==p.z);
}

void freeChunkMesh(Chunk* chunk){
	list_free(chunk->mesh);
	for(int i=0;i<4;i++){
		list_free(chunk->sideMeshes[i]);
	}
}

void initChunkMesh(Chunk* chunk){
	chunk->mesh = list_init(0);
	chunk->meshExists = true;
	for(int i=0;i<4;i++){
		chunk->sideMeshes[i]=list_init(0);
	}
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

//void reCreateWorldMesh(List* world, List* worldMesh,Player* p){
//	for(int i=0;i<(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1);i++){
//		list_free(worldMesh[i]);
//		worldMesh[i]=list_init(0);
//	}
//	createWorldMesh(world, worldMesh, p);
//}

void createChunkSideMesh(List* world,Chunk* chunk,Direction side,int chunkIndex){
	List* sideMesh = chunk->sideMeshes+side-2;
	list_free(*sideMesh);
	*sideMesh= list_init(0);
	List transparentFaces = list_init(0);
	BlockTypeEnum* blocks = (BlockTypeEnum*)chunk->blocks.l;
	for(unsigned int h=0;h<chunk->height;h++){
		for(unsigned int w=0;w<CHUNK_WIDTH;w++){
			IPos relativePos;
			switch(side){
				case West:
					relativePos = (IPos){0,h,w};
					break;
				case East:
					relativePos = (IPos){CHUNK_WIDTH-1,h,w};
					break;
				//case South:
				//	relativePos = (IPos){w,0,w};
				//	break;
				//case North:
				//	relativePos = (IPos){w,CHUNK_WIDTH-1,w};
				//	break;
				case South:
					relativePos = (IPos){w,h,0};
					break;
				case North:
					relativePos = (IPos){w,h,CHUNK_WIDTH-1};
					break;
			}
			IPos pos = ADD_IVEC(CHUNK_POS_TO_BLOCK(chunk->pos),relativePos);
			createBlockSideMesh(world,chunk,&transparentFaces,blocks[relativePos.x*CHUNK_WIDTH+relativePos.y*CHUNK_WIDTH*CHUNK_WIDTH+relativePos.z],chunkIndex,pos,side);
		}
	}
	list_concat(sideMesh,transparentFaces.used,transparentFaces.l);
	list_free(transparentFaces);
}

void createChunkMesh(List* world,Chunk* chunk,int chunkIndex){
	//ChunkPos playerChunkPos = POS_TO_CHUNK_POS(p->pos);
	initChunkMesh(chunk);
	//chunk->mesh = list_init(0);
	//for(int i=0;i<4;i++){
	//	chunk->sideMeshes[i]=list_init(0);
	//}
	List transparentFaces = list_init(0);

	//ChunkPos chunkPos = {	playerChunkPos.x+(-WORLD_CHUNKS+i/(2*WORLD_CHUNKS+1)),
	//			playerChunkPos.z+(-WORLD_CHUNKS+i%(2*WORLD_CHUNKS+1))};
	//int chunkIndex = getChunk(world,chunkPos, p->chunk_index);
	//Chunk chunk = worldl(world)[chunkIndex];
	BlockTypeEnum* blocks = (BlockTypeEnum*)chunk->blocks.l;
	for(unsigned int h=0;h<chunk->height;h++){
		for(unsigned int w=0;w<CHUNK_WIDTH*CHUNK_WIDTH;w++){
			IPos pos = ADD_IVEC(CHUNK_POS_TO_BLOCK(chunk->pos),
					((Vec){w/CHUNK_WIDTH,h,w%CHUNK_WIDTH}));
			createBlockMesh(world,chunk,&transparentFaces,
					blocks[h*CHUNK_WIDTH*CHUNK_WIDTH+w],chunkIndex,pos);
		}
	}
	//update the edges of neighbouring chunks
	for(int i=West;i<=North;i++){
		unsigned int neighbourChunkIndex =
			getChunk(world,AddIPosToChunkPos(dirVectors[i],chunk->pos),chunkIndex);
		if(neighbourChunkIndex==UINT_MAX/*||!worldl(world)[AM_CHUNKS-1].meshExists*/)
			continue;
		Chunk* neighbourChunk = &(worldl(world)[neighbourChunkIndex]);
		createChunkSideMesh(world,neighbourChunk,opposite(i),neighbourChunkIndex);
	}
	list_concat(&(chunk->mesh),transparentFaces.used,transparentFaces.l);
	list_free(transparentFaces);
}
//void createWorldMesh(List* world,List* worldMesh,Player* p){
//	for(int i=0;i<(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1);i++){
//		createChunkMesh(world,&(worldMesh[i]),p,i);
//	}
//}

void drawChunk(Chunk* chunk, Player*p){
	List *chunkMesh = &(chunk->mesh);
	for(int j=0;j<(chunkMesh->used)/sizeof(Side);j++){
		draw_face(((Side*)chunkMesh->l)[j],p);
		//draw_face(((Side*)chunkMesh->l)[j],NULL);
	}
	for(int i=0;i<4;i++){
		List* sideMesh = &(chunk->sideMeshes[i]);
		for(int j=0;j<(sideMesh->used)/sizeof(Side);j++){
			draw_face(((Side*)sideMesh->l)[j],p);
		}
	}
}

void draw_world(List* world,int* chunks_indices,Player *p){

	#define DRAW(i) drawChunk(&(worldl(world)[chunks_indices[i]]),p)
	//first draw the outer chunks
	for(int i=WORLD_CHUNKS;i>0;i--){
		int NW = (2*WORLD_CHUNKS+1)*(WORLD_CHUNKS-i)+WORLD_CHUNKS-i;
		int NE = NW+2*i;
		int SW = NW+2*i*(2*WORLD_CHUNKS+1);
		int SE = SW+2*i;

		DRAW(NW); DRAW(NE); DRAW(SW); DRAW(SE);
		//drawChunk(&(worldMesh[NW]),p);
		//drawChunk(&(worldMesh[NE]),p);
		//drawChunk(&(worldMesh[SW]),p);
		//drawChunk(&(worldMesh[SE]),p);

		for(int k=1;k<i;k++){
			DRAW(NW+k);
			DRAW(NW+(2*WORLD_CHUNKS+1)*k);

			DRAW(NE-k);
			DRAW(NE+(2*WORLD_CHUNKS+1)*k);

			DRAW(SW+k);
			DRAW(SW-(2*WORLD_CHUNKS+1)*k);

			DRAW(SE-k);
			DRAW(SE-(2*WORLD_CHUNKS+1)*k);
			
			//drawChunk(&(worldMesh[NW+k]),p);
			//drawChunk(&(worldMesh[NW+(2*WORLD_CHUNKS+1)*k]),p);
			//drawChunk(&(worldMesh[NE-k]),p);
                       //drawChunk(&(worldMesh[NE+(2*WORLD_CHUNKS+1)*k]),p);

                       //drawChunk(&(worldMesh[SW+k]),p);
                       //drawChunk(&(worldMesh[SW-(2*WORLD_CHUNKS+1)*k]),p);

                       //drawChunk(&(worldMesh[SE-k]),p);
                       //drawChunk(&(worldMesh[SE-(2*WORLD_CHUNKS+1)*k]),p);
               }
               DRAW(NW+i);
               DRAW(NE+i*(2*WORLD_CHUNKS+1));
               DRAW(SW-i*(2*WORLD_CHUNKS+1));
               DRAW(SE-i);
       }
       //Draw the chunk where the player is located
       DRAW(WORLD_CHUNKS*(2*WORLD_CHUNKS+1)+WORLD_CHUNKS);
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
	        memset(justAir,AIR,sizeof(BlockTypeEnum)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH) ;
	        list_concat(&(chunk->blocks),sizeof(BlockTypeEnum)*increaseWith*CHUNK_WIDTH*CHUNK_WIDTH,justAir);
	        chunk->height+=increaseWith;
	}
	WORLD_BLOCK_INDEX(world,index) = type;
	//Chunk* chunk = worldl(world)[chunk_index];
	freeChunkMesh(chunk);
	createChunkMesh(world,chunk,chunk_index);
}

float levelfunc(int x,int z){
//	return(0);
       return(20+perlin(((float)x)/16,((float)z)/16)*5+perlin(((float)x)/50,((float)z)/50)*30);
}

Chunk renderChunk(ChunkPos pos){
       int r = random()%(CHUNK_WIDTH*CHUNK_WIDTH);
       Chunk ret = {50,pos,list_init(50*CHUNK_WIDTH*CHUNK_WIDTH*sizeof(BlockTypeEnum))};
       memset(ret.blocks.l,AIR,ret.blocks.used);
       if(pos.x==0 && pos.z==0){
       }
       //((BlockTypeEnum*)(ret.blocks.l))[CHUNK_WIDTH]=OAK_LOG_BLOCK;
       //for(int i=0;i<CHUNK_WIDTH*CHUNK_WIDTH;i+=CHUNK_WIDTH){
       // 	((BlockTypeEnum*)(ret.blocks.l))[i]=OAK_LOG_BLOCK;
       //}

       for(int i=0;i<CHUNK_WIDTH*CHUNK_WIDTH;i++){
               int j;
               int level = levelfunc(CHUNK_WIDTH*pos.x+i/CHUNK_WIDTH,CHUNK_WIDTH*pos.z+i%CHUNK_WIDTH);
               for(j =i;j/(CHUNK_WIDTH*CHUNK_WIDTH)<level;j+=(CHUNK_WIDTH*CHUNK_WIDTH)){
                       ((BlockTypeEnum*)(ret.blocks.l))[j]=STONE_BLOCK;
               }
               for(int k=0;k<3;k++){
                       ((BlockTypeEnum*)(ret.blocks.l))[j]=DIRT_BLOCK;
                       j+=(CHUNK_WIDTH*CHUNK_WIDTH);
               }
               ((BlockTypeEnum*)(ret.blocks.l))[j]=GRASS_BLOCK;
               for(;j<WATER_LEVEL*(CHUNK_WIDTH*CHUNK_WIDTH);j+=CHUNK_WIDTH*CHUNK_WIDTH){
                       ((BlockTypeEnum*)(ret.blocks.l))[j]=WATER_BLOCK;
               }
               if(i==r&&level>WATER_LEVEL){
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

void loadChunks(List* world,int* visibleChunks,Player p){
       int prevAmChunks = AM_CHUNKS;
       ChunkPos playerChunkPos = POS_TO_CHUNK_POS(p.pos);
       for(int i=0;i<(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1);i++){
               ChunkPos chunkPos = {playerChunkPos.x+(-WORLD_CHUNKS+i/(2*WORLD_CHUNKS+1)),
                                    playerChunkPos.z+(-WORLD_CHUNKS+i%(2*WORLD_CHUNKS+1))};

               int chunkIndex = getChunk(world,chunkPos, p.chunk_index);
               if(chunkIndex==UINT_MAX){
                       list_append(world,sizeof(Chunk));
                       worldl(world)[AM_CHUNKS-1]=renderChunk(chunkPos);
               }
               chunkIndex = getChunk(world,chunkPos, p.chunk_index);
               visibleChunks[i]=chunkIndex;
       }
       for(int j=prevAmChunks; j<AM_CHUNKS;j++){
               createChunkMesh(world, &(worldl(world)[j]), j);
       }
}

