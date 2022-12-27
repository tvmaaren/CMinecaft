default: minecraft

libs=-lallegro -lallegro_font -lallegro_primitives -lm -lallegro_image -lallegro_dialog
CPPFLAGS=$(libs) -Wfatal-errors -g

minecraft: minecraft.c list.o
list.o: list.c
