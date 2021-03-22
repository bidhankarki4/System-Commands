newshell: major2.o
	gcc -o newshell major2.c 
major2.o: 	
	gcc -c major2.c -lreadline
clean:
	rm *o newshell
run:
	./newshell