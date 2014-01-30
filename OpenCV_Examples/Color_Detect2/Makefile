CC = g++
CFLAGS = -g -Wall
SRCS = objectTrackingTut.cpp
PROG = objectTrack
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
