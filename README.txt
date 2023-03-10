• The project team members' names and division of labor
	Mariela Badillo
	Miguel Montesinos
	Haleigh Gahan
	 
• List who worked on each part of the project
	Part 1: Mariela Badillo and Miguel Montesinos and Haleigh Gahan
	Part 2: Mariela Badillo and Miguel Montesinos and Haleigh Gahan
	Part 3:
		a Kernel Module with a Bar: Mariela Badillo and Miguel Montesinos and Haleigh Gahan
		b Add System Calls: Mariela Badillo and Miguel Montesinos and Haleigh Gahan
		c /Proc: Mariela Badillo and Miguel Montesinos and Haleigh Gahan
		d Test: Mariela Badillo and Miguel Montesinos and Haleigh Gahan

• A list of files in your repository and a brief description of each file
	Part1:
		empty.c: empty program
		empty.trace: a trace of the calls of empty.c It should be 4 lines shorter because it has 4 calls less.
		part1.c: program with 4 calls
		part1.trace: a trace of the calls of part1.c. It should be 4 lines longer because it has 4 calls more(fork()).
		Makefile: compiles empty and part1 with the make command inside the folder that contains both of them.
	
	Part2:
		my_timer.c: creates a kernel module that tracks the time elapsed since the mod was inserted.
		Makefile: compiles the kernel module in the folder with the name my_timer into a my_timer.ko

	Part3:
		barstool.c: body of part3. Consist of all source code used for the scheduling bar
		sys_call.c: defines the system calls used in barstool.c
		Makefile: compiles barstool.c into the kernel module barstool.ko so that it can be inserted.

• How to compile your executables using your Makefile
	Go into the folder containing the file to be executed and use the command make.

• Known bugs and unfinished portions of the project 
	No known bugs
