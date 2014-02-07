CC = g++
CFLAGS = -g -Wall
SRCS = QRGPS.cpp
PROG = QRGPS
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) 
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) -lzbar $(LIBS)

test: $(PROG)
	./$(PROG) qrcode1.jpg
