include ${MOAB_MAKE}

CC = g++

CPPFLAGS += ${MOAB_INCLUDES} -g
CFLAGS += ${MOAB_CFLAGS}



all: slicer 

slicer: 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c slicer.cpp -o slicer.o


clean: 
	rm -f *.o *~
