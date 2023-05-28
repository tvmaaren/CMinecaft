#default: minecraft
#
#libs=-lallegro -lallegro_font -lallegro_primitives -lm -lallegro_image -lallegro_dialog
#CFLAGS=$(libs) -Wfatal-errors -g3 -gdwarf-2
#
##main:  main.c minecraft.o
#minecraft: minecraft.c list.o world.o block.o
#world.o: world.c list.o block.o world.h
#block.o: block.c block.h
#list.o: list.c list.h


default: main

libs=-lallegro -lallegro_font -lallegro_ttf -lallegro_primitives -lm -lallegro_image -lallegro_dialog
CFLAGS=$(libs)  -g3 -gdwarf-2

main: main.c minecraft.o world.o block.o list.o gui.o
gui: gui.c gui.h
minecraft.o: minecraft.c list.o world.o block.o
world.o: world.c list.o block.o world.h
block.o: block.c block.h
list.o: list.c list.h

clean: 
	rm *.o main
