
#define SIZE_BLOCK 8

typedef enum {DIRT=0, GRASS_BLOCK_SIDE=1, GRASS_BLOCK_TOP=2, COBBLESTONE=3,
	OAK_PLANKS=4, OAK_LOG=5, OAK_LOG_TOP=6,STONE=7,OAK_LEAVES=8,GLASS=9,WATER=10} Texture;


typedef struct{
		  Texture texture;
		  Color color;
}BlockSidePreProcessing;

//0:top, 1:bottem, 2:north, 3:west, 4:east, 5:south
typedef struct{
       	BlockSidePreProcessing sides[6];
	bool transparent;
}BlockTypePreProcessing;

typedef struct{
		  ALLEGRO_BITMAP* texture;
		  ALLEGRO_COLOR color;
}BlockSide;

//0:top, 1:bottem, 2:north, 3:west, 4:east, 5:south
//typedef BlockSide BlockType[6];
typedef struct{
	BlockSide sides[6];
	bool transparent;
}BlockType;

typedef enum{AIR=UINT_MAX, GRASS_BLOCK=0, DIRT_BLOCK=1, COBBLESTONE_BLOCK=2,
	OAK_PLANKS_BLOCK=3, OAK_LOG_BLOCK=4, STONE_BLOCK=5,OAK_LEAVES_BLOCK=6,GLASS_BLOCK=7,WATER_BLOCK=8}BlockTypeEnum;


typedef enum{Top=0,Bottem=1,West=2,East=3,South=4,North=5,NoDirection} Direction;

typedef struct{
	BlockTypeEnum type;
	Pos pos;
	Direction side;
}Side;

void draw_face(Side face, Player *p);
void draw_block(List* world,List* transparentBlocks, BlockTypeEnum type,unsigned int chunk_index,Pos pos,Player* player);
Pos adjacentPos(Pos p,  Direction d);
void loadBlocks();
Direction hitDirectionBlock(Pos prev_pos, Pos block_pos);
void createBlockMesh(List* world,List* chunkMesh,List* transparentBlocks, BlockTypeEnum type,unsigned int chunk_index,Pos pos);
