#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <stdio.h>

#include "gui.h"

extern ALLEGRO_DISPLAY* display;
extern int screen_width,screen_height;
#define w screen_width
#define h screen_height
extern const char fontPath[];

int min(int a,int b){
	return a<b ? a:b;
}

void drawFiledRect(Rect r){
	al_draw_filled_rectangle(r.x1*w, r.y1*h, r.x2*w, r.y2*h,
			al_map_rgb(0,255,0));

}
void drawRect(Rect r){
	al_draw_rectangle(r.x1*w, r.y1*h, r.x2*w, r.y2*h,
			al_map_rgb(255,0,0),10);

}

void drawTextInRect(Rect r, char* string){
	int middle_x = (r.x1+r.x2)/2.0*w;
	int middle_y = (r.y1+r.y2)/2.0*h;
	int size = min(h*(r.y2-r.y1),w/strlen(string));
	ALLEGRO_FONT* font = al_load_ttf_font(fontPath,size,0);
	al_draw_text(font,al_map_rgb(255,255,255),middle_x,middle_y-size/2,ALLEGRO_ALIGN_CENTRE,string);
}

bool inRect(Rect r, int x, int y){
	return(r.x1*w<=x && r.x2*w>=x && r.y1*h<=y && r.y2*h>=y);
}

void handleButton(Button* button, ALLEGRO_EVENT* event){
	switch(event->type){
		case(ALLEGRO_EVENT_MOUSE_AXES):
			button->hover = inRect(button->rect,event->mouse.x,event->mouse.y);
			if(!button->hover)button->click=false;
			break;
		case(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN):
			if(event->mouse.button!=1)break;
			button->click = inRect(button->rect,event->mouse.x,event->mouse.y);
			break;
		case(ALLEGRO_EVENT_MOUSE_BUTTON_UP):
			if(button->click){
				button->click=false;
				button->action();
			}
			break;

	}
}
void drawButton(Button* button){
	if(button->hover)drawFiledRect(button->rect);
	drawRect(button->rect);
	drawTextInRect(button->rect,button->text);
}

void initButton(Button* button,char* text,float x1,float y1,float x2,float y2,void (*action)(void)){
	button->text = text;
	button->hover=false;
	button->click=false;
	button->rect.x1=x1;
	button->rect.y1=y1;
	button->rect.x2=x2;
	button->rect.y2=y2;
	button->action = action;
}

void handleGui(Gui* gui, ALLEGRO_EVENT* e){
	for(int i=0;i<gui->n;i++){
		if(gui->types[i]==BUTTON)handleButton((Button*)gui->widgets[i],e);
	}
}

void drawGui(Gui* gui){
	for(int i=0;i<gui->n;i++){
		if(gui->types[i]==BUTTON)drawButton((Button*)gui->widgets[i]);
	}
}
void initGui(Gui* gui,void** widgets, WidgetType* types, int n){
	gui->widgets =widgets;
	gui->types = types;
	gui->n=n;
}
