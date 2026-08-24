CC=arm-linux-gcc
#DEG=-DDEFINE_SUZHOU_XIAN=1
#R3018=-DREADER_3018=1

1:liblinux4418_493.so
liblinux4418_493.so: global.o 
	$(CC)  -shared -o liblinux4418_493.so global.o

2:liblinux4418_493.a
liblinux4418_493.a: global.o
	ar -rc liblinux4418_493.a global.o

global.o:global.c 
	$(CC) -c $(DEG) $(R3018) global.c -o global.o -fPIC

clean:
	rm -f *.o liblinux4418_493.so liblinux4418_493.a