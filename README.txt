Kaitlynn Whitney, Kieron Yin, Bidhan Karki

How to compile:
	make
	
How to run:
	./newshell [batchfile]
	
Note: the batchfile is optional.

Organization of the Project:
Kieron Yin (kty0012 - leader): (1)built-in cd command, (3)support signal handling and terminal control (incomplete), README
Kaitlynn Whitney (kaw0394): main, (2)exit command, (2)extending shell with pipelining, makefile, README
    Note: The pipeline function will run all arguments without a pipe prior to the piped arguments. 
    There can only be one pipe argument per line.

Known Issues:
 - Occasionally the file will not execute properly the first time it is ran for interactive mode
	-Please 'make clean' compile and run again
 - If a line has a pipe argument, all other arguments on that line will execute prior to the pipe
 - Occassionally cd will require you to reset the shell during runtime
 - Implementation of signal handling is incomplete, functions declared and in process of implimentation but left out due to compile errors

Built-in commands NOT included in code:
 - Adding a new built-in path
 - Adding a new built-in history
 
Support NOT included in code:s
 - Extending shell with I/O redirection
 - Implement a new alias command


Bidhan Karki
Just completed a new built-in history and extending the shell with I/O redirection.
