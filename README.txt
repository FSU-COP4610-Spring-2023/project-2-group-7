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
		empty.trace: a trace of the system calls of empty.c It should be 4 lines shorter because it has 4 calls less.
		part1.c: program with 4 system calls
		part1.trace: a trace of the system calls of part1.c. It should be 4 lines longer because it has 4 more calls(getpid()).
		Makefile: compiles empty.c and part1.c. To use run the 'make' command inside the folder that contains both files.
	
	Part2:
		my_timer.c: creates a kernel module that tracks the time elapsed since the mod was inserted.
		Makefile: compiles the kernel module in the file my_timer.c. To use run the 'make' command inside the folder that contains the file.
		This creates my_timer.ko which can be inserted using the command 'sudo insmod my_timer.ko', this creates a proc entry named timer.
		To remove the module run the command 'sudo rmmod my_timer', then the proc entry titled timer will be deleted.

	Part3:
		barstool.c: body of part3. Consists of all of the source code used for the bar scheduler
		sys_call.c: defines the 3 system calls creaed for use in barstool.c
		Makefile: compiles barstool.c into the kernel module barstool.ko so that it can be inserted. To use run the 'make' command inside the
		folder that contains the file. This creates barstool.ko which can be inserted using the command 'sudo insmod barstool.ko', this creates
		a proc entry named majorsbar. To remove the module run the command 'sudo rmmod barstool', then the proc entry titled majorsbar will be
		removed.

• How to compile your executables using your Makefile
	Go into the folder containing the file to be executed and use the command make.

• Known bugs and unfinished portions of the project 
	No known bugs