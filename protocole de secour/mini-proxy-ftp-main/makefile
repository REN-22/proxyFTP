CC=gcc

main: proxy
	./proxy

trace: proxy
	strace ./proxy

proxy: proxy.o fonctions.o simpleSocketAPI.o
	${CC} $^ -o $@ 

proxy.o: proxy.c fonctions.h simpleSocketAPI.h
	${CC} -c $^ 

fonctions.o: fonctions.c simpleSocketAPI.h
	${CC} -c $^ 

simpleSocketAPI.o: simpleSocketAPI.c
	${CC} -c $^ 

clean:
	rm -rf *.o *.gch proxy
