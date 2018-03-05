FLAGS=-O2 -Wall

clean:
	rm bin/*

all: control modules

control: src/main.c rx tx input
	gcc $(FLAGS) src/main.c bin/rx.o bin/tx.o bin/input.o -lm -ldl -lpthread -lconfig -o bin/control

rx: src/reciver.c
	gcc $(FLAGS) -c src/reciver.c -o bin/rx.o

tx: src/transmitter.c
	gcc $(FLAGS) -c src/transmitter.c -o bin/tx.o

input: src/input.c
	gcc $(FLAGS) -c src/input.c -o bin/input.o

modules: elevon debug
	

elevon: src/modules/elevon.c
	gcc $(FLAGS) -fPIC -g -c src/modules/elevon.c -o bin/elevon.o
	gcc -shared -Wl,-soname,elevon.so.1 -o bin/elevon.so.1.0 bin/elevon.o -lc
	mv bin/elevon.so.1.0 /usr/lib
	ln -sf /usr/lib/elevon.so.1.0 /usr/lib/elevon.so.1
	ln -sf /usr/lib/elevon.so.1.0 /usr/lib/elevon.so
	ldconfig

debug: src/modules/debug.c
	gcc $(FLAGS) -fPIC -g -c src/modules/debug.c -o bin/debug.o
	gcc -shared -Wl,-soname,debug.so.1 -o bin/debug.so.1.0 bin/debug.o -lc
	mv bin/debug.so.1.0 /usr/lib
	ln -sf /usr/lib/debug.so.1.0 /usr/lib/debug.so.1
	ln -sf /usr/lib/debug.so.1.0 /usr/lib/debug.so
	ldconfig
