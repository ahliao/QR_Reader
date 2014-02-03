CC = g++
CFLAGS = -g -Wall
SRCS = ZBar_Test.cpp
PROG = ZBar_Test
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) 
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) -lzbar $(LIBS)
