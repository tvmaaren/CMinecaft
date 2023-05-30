typedef struct{
	float x,y,z;
}Pos;
typedef struct{
	int x,y,z;
}IPos;
typedef Pos Vec;
typedef enum{Top=0,Bottem=1,West=2,East=3,South=4,North=5,NoDirection} Direction;
#define opposite(dir) 1-(dir%2)*2+dir

//Vec dirVectors[6]={{0,1,0},{0,-1,0},{-1,0,0},{1,0,0},{0,0,-1},{0,0,1}};
typedef struct{
		  uint8_t r,g,b;
}Color;

typedef struct{
	float hor_angle;
	float vert_angle;
	Pos pos;
	unsigned int chunk_index;
	bool flying;
	bool falling;
	float vert_speed;//only used when falling
	unsigned int standing_on;
}Player;

#define ADD_VEC(v,w) (Pos){v.x+w.x,v.y+w.y,v.z+w.z}
#define ADD_IVEC(v,w) (IPos){v.x+w.x,v.y+w.y,v.z+w.z}
#define SCALAR_MUL_VEC(l,v) (Vec){l*v.x,l*v.y,l*v.z}
#define CAST_TO_POS(v) (Vec){(float)v.x,(float)v.y,(float)v.z}
