//Amount of Chunks that should be loaded in every
//direction

#define WORLD_CHUNKS 4

#define CHUNK_WIDTH 		16 //Depth and width of a chunk
#define MAX_CHUNK_HEIGHT 	256
#define MAX_CHUNK_BlOCKS 	(CHUNK_WIDTH*CHUNK_WIDTH*MAX_CHUNK_HEIGHT)

#define worldl(w) ((Chunk*)(w)->l)
#define WORLD_BLOCK_INDEX(w,i)  ((BlockTypeEnum*)(worldl(w)[i/MAX_CHUNK_BlOCKS].blocks.l))[i%MAX_CHUNK_BlOCKS]
#define FLOOR_I(f) (unsigned int)floorf(f)
#define POS_TO_BLOCK_INDEX(pos) FLOOR_I(pos.x)%CHUNK_WIDTH*CHUNK_WIDTH + FLOOR_I(pos.y)*CHUNK_WIDTH*CHUNK_WIDTH + FLOOR_I(pos.z)%CHUNK_WIDTH
#define CHUNK_POS_TO_BLOCK(chunk) (Pos){CHUNK_WIDTH*chunk.x,0,CHUNK_WIDTH*chunk.z}
#define POS_TO_CHUNK_POS(pos) (ChunkPos){FLOOR_I(pos.x/CHUNK_WIDTH),FLOOR_I(pos.z/CHUNK_WIDTH)}
#define INDEX_TO_POS(w,i)  ((Pos)ADD_VEC((CHUNK_POS_TO_BLOCK(worldl(w)[i/MAX_CHUNK_BlOCKS].pos)),((Vec){i%(CHUNK_WIDTH*CHUNK_WIDTH)/CHUNK_WIDTH,i%MAX_CHUNK_BlOCKS/(CHUNK_WIDTH*CHUNK_WIDTH),i%CHUNK_WIDTH})))
#define POS_TO_INDEX(world,pos,chunk) (getChunk(world,(POS_TO_CHUNK_POS(pos)),chunk)*MAX_CHUNK_BlOCKS+POS_TO_BLOCK_INDEX(pos))


void saveWorld(FILE* restrict stream, List* world);
void loadWorld(FILE* restrict stream, List* world);
bool isCorrectChunk(List* world, ChunkPos p, unsigned int chunk_index);
unsigned int getChunk(List* world, ChunkPos p, unsigned int i);
unsigned int is_block(List* world,Pos pos, unsigned int chunk_index);
void draw_world(List* world,int* chunks_indices,Player *p);
void setBlock(List* world, int index, int chunk_index,BlockTypeEnum type);
Chunk renderChunk(ChunkPos pos);
void loadChunks(List* world,int* visibleChunks,Player p);
void createWorldMesh(List* world,List* worldMesh,Player* p);
void reCreateWorldMesh(List* world,List* worldMesh,Player* p);
void createChunkMesh(List* world,Chunk* chunk,int chunkIndex);
void freeChunkMesh(Chunk* chunk);
