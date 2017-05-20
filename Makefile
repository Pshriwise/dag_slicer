include ${MOAB_MAKE}

CC = g++

INCLUDES += ${MOAB_INCLUDES}
CFLAGS += ${MOAB_CFLAGS}
CFLAGS += -std=c++11


all: slicer model_slicer

dag_slicer: 
	$(CC) $(CFLAGS) $(INCLUDES) -c src/dag_slicer.cpp -o dag_slicer.o

slicer: 
	$(CC) $(CFLAGS) $(INCLUDES) -c src/slicer.cpp -o slicer.o

model_slicer: slicer dag_slicer
	$(CC) $(CFLAGS) $(INCLUDES) src/model_slicer.cpp slicer.o dag_slicer.o -o model_slicer ${MOAB_LIBS_LINK} 

debug: clean
	$(CC) $(INCLUDES) ${MOAB_LIBS_LINK}  -g -c src/dag_slicer.cpp -o dag_slicer.o

	$(CC) $(INCLUDES) ${MOAB_LIBS_LINK} -g -c src/slicer.cpp -o slicer.o

	$(CC) $(INCLUDES)  -g src/model_slicer.cpp slicer.o dag_slicer.o -o model_slicer -I src/slicer.hpp ${MOAB_LIBS_LINK}

clean: 
	rm -f *.o *~ model_slicer slicepnts.txt
