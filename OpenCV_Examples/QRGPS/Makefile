CC = g++
CFLAGS = -g -Wall
SRCS = QRGPS.cpp
PROG = QRGPS
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) 
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) -lzbar $(LIBS)

test: $(PROG)
	./$(PROG) angle_38.jpg
	./$(PROG) angle_45.jpg
	./$(PROG) angle_80.jpg
	./$(PROG) angle_120.jpg
	./$(PROG) angle_150.jpg
	./$(PROG) angle_170.jpg
	./$(PROG) angle_190.jpg
	./$(PROG) angle_210.jpg
	./$(PROG) angle_260.jpg
	./$(PROG) angle_320.jpg
	./$(PROG) angle_330.jpg
