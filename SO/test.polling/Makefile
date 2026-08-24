CC=arm-linux-gcc
#CC=/usr/local/arm/4.3.2/bin/arm-linux-gcc 

OBJS=main.o polling.o

LIBS=-L. -llinux4418_493 -lpthread -lm

1:polling
polling: $(OBJS)
	$(CC)  -o polling $(OBJS) $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o

polling.o: polling.c
	$(CC) -c polling.c -o polling.o
	
clean:
	rm -f *.o polling
