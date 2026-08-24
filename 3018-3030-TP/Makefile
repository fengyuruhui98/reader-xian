CC=arm-linux-gcc
OBJS=main.o md5.o sz_xdr_api.o serial.o crc.o xdr_file_manage.o time_tools.o xa_ul_operation.o fastdes.o xa_func.o xa_cpu20_operation.o xa_sam.o hard_2440_test.o hh_cpu_operation.o xa_tong_operation.o bin_file_manage.o xa_operation.o sha1.o  xa_pboc_operation.o tlv.o xa_transport_operation.o xa_qr_operation.o 
# for .a or .so:-L libpath  -llibname  sz_m1_operation.o  sz_tong_operation.o  xa_m1_operation.o structbit.o 
LIBS= -lpthread  
#-lrt -fpack-struct -DDEBUG_PRINT=1
#DEG=-DDEBUG_PRINT=1

#定义车站到车站的票价
#DEBUG_STATION_PRICE=1

1:xian
xian: $(OBJS) 
	$(CC)  -o xian $(OBJS)  $(LIBS) -L. -llinux2440
#xdr.o xdr_array.o xdr_float.o xdr_intXX_t.o xdr_mem.o xdr_rec.o xdr_ref.o xdr_sizeof.o xdr_stdio.o

2:xian_4418
xian_4418: $(OBJS) xa_name.o
	$(CC)  -o xian_4418 $(OBJS) xa_name.o  $(LIBS) -L. -llinux4418_493 

#	$(CC)  -o xian_4418 $(OBJS)  $(LIBS) -L. -Wl,-Bdynamic -llinux4418_493 

main.o: main.c
	$(CC) -c main.c -o main.o

fastdes.o: fastdes.c fastdes.h
	$(CC) -c fastdes.c -o fastdes.o
	
md5.o: md5.c md5.h
	$(CC) -c md5.c -o md5.o

bin_file_manage.o:bin_file_manage.c bin_file_manage.h binEOD.h
	$(CC)  -c $(DEG) -fpack-struct bin_file_manage.c -o bin_file_manage.o

xdr_file_manage.o:xdr_file_manage.c xdr_file_manage.h
	$(CC) -c  xdr_file_manage.c -o xdr_file_manage.o

xa_operation.o:xa_operation.c xa_operation.h binEOD.h
	$(CC) -c  $(DEG) xa_operation.c -o xa_operation.o

xa_transport_operation.o:xa_transport_operation.c xa_transport_operation.h
	$(CC) -c  $(DEG) xa_transport_operation.c -o xa_transport_operation.o
	
xa_tong_operation.o:xa_tong_operation.c xa_tong_operation.h binEOD.h
	$(CC) -c  $(DEG) xa_tong_operation.c -o xa_tong_operation.o
	
xa_cpu20_operation.o:xa_cpu20_operation.c xa_cpu20_operation.h binEOD.h
	$(CC) -c  $(DEG) xa_cpu20_operation.c -o xa_cpu20_operation.o
	
xa_ul_operation.o:xa_ul_operation.c xa_ul_operation.h binEOD.h
	$(CC) -c  $(DEG) xa_ul_operation.c -o xa_ul_operation.o
		
serial.o: serial.c serial.h
	$(CC) -c  $(DEG) serial.c -o serial.o

crc.o: crc.c crc.h
	$(CC) -c crc.c -o crc.o

time_tools.o:time_tools.c time_tools.h
	$(CC) -c  $(DEG) time_tools.c -o time_tools.o

xa_func.o:xa_func.c xa_func.h
	$(CC) -c  $(DEG) xa_func.c -o xa_func.o 

xa_sam.o:xa_sam.c xa_sam.h
	$(CC) -c  $(DEG) xa_sam.c -o xa_sam.o

hard_2440_test.o:hard_2440_test.c
	$(CC) -c  $(DEG) hard_2440_test.c -o hard_2440_test.o

hh_cpu_operation.o:hh_cpu_operation.c
	$(CC) -c  $(DEG) hh_cpu_operation.c -o hh_cpu_operation.o

sha1.o:sha1.c sha1.h
	$(CC) -c sha1.c -o sha1.o

xa_pboc_operation.o:xa_pboc_operation.c
	$(CC) -c  $(DEG) xa_pboc_operation.c -o xa_pboc_operation.o

xa_qr_operation.o:xa_qr_operation.c 
	$(CC) -c $(DEG) xa_qr_operation.c -o xa_qr_operation.o

xa_name.o:xa_name.c xa_name.h
	$(CC) -c $(DEG) xa_name.c -o xa_name.o
	
tlv.o:tlv.c
	$(CC) -c tlv.c -o tlv.o

#test follow
structbit.o:structbit.c
	$(CC) -c  structbit.c -o structbit.o

xa_m1_operation.o:xa_m1_operation.c xa_m1_operation.h
	$(CC) -c  xa_m1_operation.c -o xa_m1_operation.o

clean:
	rm -f *.o