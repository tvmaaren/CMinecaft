typedef struct{
	float x,y,z;
}Pos;
typedef Pos Vec;
typedef struct{
		  uint8_t r,g,b;
}Color;

typedef struct{
	float hor_angle;
	float vert_angle;
	Pos pos;
	unsigned int chunk_index;
	bool falling;
	float vert_speed;//only used when falling
}Player;

#define ADD_VEC(v,w) (Pos){v.x+w.x,v.y+w.y,v.z+w.z}
#define SCALAR_MUL_VEC(l,v) (Vec){l*v.x,l*v.y,l*v.z}
