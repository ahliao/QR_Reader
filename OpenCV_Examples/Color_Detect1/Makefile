CC = g++
CFLAGS = -g -Wall
SRCS = detect.cpp
PROG = detect
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
