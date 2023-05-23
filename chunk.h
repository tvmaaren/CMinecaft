typedef struct{
	int x,z;
}ChunkPos;

#define AddIPosToChunkPos(ipos,chunkPos) (ChunkPos){ipos.x+chunkPos.x,ipos.z+chunkPos.z}

typedef struct{
	unsigned int height;
	ChunkPos pos;
	List blocks;
	bool meshExists;
	List mesh;
	List sideMeshes[4];
}Chunk;
