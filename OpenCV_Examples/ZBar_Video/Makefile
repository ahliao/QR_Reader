CC = g++
CFLAGS = -g -Wall
SRCS = ZBar_Video.cpp
PROG = ZBar_Video
 
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) 
	 
$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) -lzbar $(LIBS)

test: $(PROG)
	./$(PROG) qrcode1.jpg
	./$(PROG) qrcode2.jpg
	./$(PROG) qrcode3.jpg
	./$(PROG) qrcode4.jpg
	./$(PROG) qrcode5.jpg
	./$(PROG) qrcode6.jpg
	./$(PROG) qrcode7.jpg
	./$(PROG) qrcode8.jpg
	./$(PROG) qrcode9.jpg
