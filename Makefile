default: minecraft

libs=-lallegro -lallegro_font -lallegro_primitives -lm -lallegro_image -lallegro_dialog
CPPFLAGS=$(libs) -Wfatal-errors -g

minecraft: minecraft.c list.o world.o block.o
world.o: world.c list.o block.o
block.o: block.c
list.o: list.c
