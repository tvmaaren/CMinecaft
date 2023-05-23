default: minecraft

libs=-lallegro -lallegro_font -lallegro_primitives -lm -lallegro_image -lallegro_dialog
CFLAGS=$(libs) -Wfatal-errors -g3 -gdwarf-2

minecraft: minecraft.c list.o world.o block.o
world.o: world.c list.o block.o world.h
block.o: block.c block.h
list.o: list.c list.h
