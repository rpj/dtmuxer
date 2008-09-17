all:
	$(CC) -o dtMuxer dtM4AProc.c dtM4ARead.c dtM4AUtil.c dtM4AWrite.c main.c

clean:
	rm dtMuxer
