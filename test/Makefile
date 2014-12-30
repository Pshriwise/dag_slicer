include ${MOAB_MAKE}

CC = g++

CPPFLAGS += ${MOAB_INCLUDES} -g
CFLAGS += ${MOAB_CFLAGS}


all: test_slicer

test_slicer: 
	$(CC) $(CFLAGS) $(CPPFLAGS) -I ../src/ test_slicer.cpp ../src/slicer.o -o test_slicer \
	${MOAB_LIBS_LINK}

check: test_slicer
	./test_slicer

clean: 
	rm -f *.o *~ test_slicer
