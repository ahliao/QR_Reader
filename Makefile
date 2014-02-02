CC = g++
CFLAGS = -g -Wall
SRCS = QR_Reader.cpp
PROG = QR_Reader
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) 
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) -lzbar $(LIBS)
