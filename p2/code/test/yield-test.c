/*
 * yield-test.c
 */

#include <syscall.h>

void print(char *s)
{
	int len = 0;

	while (*s++)
		++len;

	Write(s-len-1, len, 1);
}

void proc3()
{
	print("proc3 Yielding...\n");
	Yield();
	print("proc2 resuming...\n");
	print("Preparing to Halt...\n");
	Halt();
}

void proc2()
{
	print("proc2 Yielding...\n");
	Yield();
	print("proc2 resuming...\n");
	Exit(0);
}

void proc1()
{
	print("proc1 Yielding...\n");
	Yield();
	print("proc1 resuming...\n");
	Exit(0);
}

main()
{
	print("main Forking some stuff...\n");
	Fork(proc1);
	Fork(proc2);
	Fork(proc3);
}
