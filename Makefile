all:
	gcc -c solution.c
	gcc solution.c -o solution -pthread -lm

clean:
	rm solution *.o 
