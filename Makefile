CC=g++
CFLAGS=-Wall -O3 -framework OpenCL
TARGET=app.out
OBJS=OpenCLInitHelper.o\
	Main.o
	
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	find . -name "*.o" -exec rm -f {} \;
	find . -name "*.out" -exec rm -f {} \;