include ${MOAB_MAKE}

CC = g++

CPPFLAGS += ${MOAB_INCLUDES} -g
CFLAGS += ${MOAB_CFLAGS}



all: slicer model_slicer

dag_slicer: 
	$(CC) $(CFLAGS) $(CPPFLAGS) ${MOAB_LIBS_LINK} -c dag_slicer.cpp -o dag_slicer.o

slicer: 
	$(CC) $(CFLAGS) $(CPPFLAGS) ${MOAB_LIBS_LINK} -c slicer.cpp -o slicer.o

model_slicer: slicer dag_slicer
	$(CC) $(CFLAGS) $(CPPFLAGS)  model_slicer.cpp slicer.o dag_slicer.o -o model_slicer -I slicer.hpp ${MOAB_LIBS_LINK}

clean: 
	rm -f *.o *~ model_slicer slicepnts.txt
