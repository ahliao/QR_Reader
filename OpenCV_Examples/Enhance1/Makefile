CC = g++
CFLAGS = -g -Wall
SRCS = Enhance1.cpp
PROG = Enhance1
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) 
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) -lzbar $(LIBS)

test: $(PROG)
	./$(PROG) small1.jpg
