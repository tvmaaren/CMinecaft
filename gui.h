typedef enum{BUTTON=0} WidgetType;


typedef struct{
	float x1,y1,x2,y2;//between 0 and 1
}Rect;

typedef struct{
	char* text;
	bool hover;
	bool click;
	Rect rect;
	void (*action)(void);
}Button;

typedef struct{
	void** widgets;
	WidgetType* types;
	int n;
}Gui;
void drawButton(Button* button);
void initButton(Button* button,char* text,float x1,float y1,float x2,float y2,void (*action)(void));
void handleGui(Gui* gui, ALLEGRO_EVENT* e);
void drawGui(Gui* gui);
void initGui(Gui* gui,void** widgets, WidgetType* types, int n);
