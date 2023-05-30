
typedef struct{
	List world;
	int visibleChunks[(2*WORLD_CHUNKS+1)*(2*WORLD_CHUNKS+1)];
	Player player;
	int hotbarSelect;
	BlockTypeEnum hotbar[9];
	BlockTypeEnum block_selection;//TODO: See if this is necessary
	unsigned char key[ALLEGRO_KEY_MAX];
}Minecraft;


void drawMinecraft(Minecraft* minecraft);
void handleMinecraft(Minecraft* minecraft, ALLEGRO_EVENT* event);
void initMinecraft(Minecraft* minecraft);
