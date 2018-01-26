#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h> 
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/*********************************************************************
 *
 * These C functions use patterns and functionality often found in
 * operating system code. Your job is to implement them. Of course you
 * should write test cases. However, do not hand in your test cases
 * and (especially) do not hand in a main() function since it will
 * interfere with our tester.
 *
 * Additional requirements on all functions you write:
 *
 * - you may not refer to any global variables
 *
 * - you may not call any functions except those specifically
 *   permitted in the specification
 *
 * - your code must compile successfully on CADE lab Linux
 *   machines when using:
 *
 * /usr/bin/gcc -O2 -fmessage-length=0 -pedantic-errors -std=c99 -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter -Wshadow -Wuninitialized -Wold-style-definition -c assign1.c 
 *
 * NOTE 1: Some of the specifications below are specific to 64-bit
 * machines, such as those found in the CADE lab.  If you choose to
 * develop on 32-bit machines, some quantities (the size of an
 * unsigned long and the size of a pointer) will change. Since we will
 * be grading on 64-bit machines, you must make sure your code works
 * there.
 *
 * NOTE 2: You should not need to include any additional header files,
 * but you may do so if you feel that it helps.
 *
 * HANDIN: submit your finished file, still called assign1.c, in Canvas
 *
 *
 *********************************************************************/

/*********************************************************************
 *
 * byte_sort()
 *
 * specification: byte_sort() treats its argument as a sequence of
 * 8 bytes, and returns a new unsigned long integer containing the
 * same bytes, sorted numerically, with the smaller-valued bytes in
 * the lower-order byte positions of the return value
 * 
 * EXAMPLE: byte_sort (0x0403deadbeef0201) returns 0xefdebead04030201
 *
 *********************************************************************/

unsigned long byte_sort (unsigned long arg)
{
	int byte_number = 0;
	for (unsigned long temp = arg; temp != 0; temp >>= 8)
		++byte_number;
	int *numbers = (int*)calloc(byte_number, sizeof(int));
	if (numbers == NULL)
		exit(-1);
	int index = 0;
	for (unsigned long temp = arg; temp != 0; temp >>= 8)
		numbers[index++] = temp & ((1 << 8) - 1);
	for (int i = 0; i < byte_number; ++i)
		for (int j = i + 1; j < byte_number; ++j)
			if (numbers[i] > numbers[j])
			{
				int temp = numbers[i];
				numbers[i] = numbers[j];
				numbers[j] = temp;
			}
				
	unsigned long ans = 0;
	for (index = byte_number - 1; index >= 0; --index)
		ans = (ans << 8) + numbers[index];
	free(numbers);
	return ans;
}

/*********************************************************************
 *
 * nibble_sort()
 *
 * specification: nibble_sort() treats its argument as a sequence of 16 4-bit
 * numbers, and returns a new unsigned long integer containing the same nibbles,
 * sorted numerically, with smaller-valued nibbles towards the "small end" of
 * the unsigned long value that you return
 *
 * the fact that nibbles and hex digits correspond should make it easy to
 * verify that your code is working correctly
 * 
 * EXAMPLE: nibble_sort (0x0403deadbeef0201) returns 0xfeeeddba43210000
 *
 *********************************************************************/

unsigned long nibble_sort (unsigned long arg)
{
	int* numbers = (int*)calloc(16, 4);
	int index = 0;
	for (unsigned long temp = arg; temp > 0; temp >>= 4, ++index)
		numbers[index] = temp & ((1 << 4) - 1);
	
	for (int i = 0; i < 16; ++i)
		for (int j = i + 1; j < 16; ++j)
			if (numbers[i] < numbers[j])
			{
				int temp = numbers[i];
				numbers[i] = numbers[j];
				numbers[j] = temp;
			}
				
	unsigned long ans = 0;
	for (int i = 0; i < 16; ++i)
		ans = (ans << 4) + numbers[i];
	free(numbers);
	return ans;
}

/*********************************************************************
 *
 * name_list()
 *
 * specification: allocate and return a pointer to a linked list of
 * struct elts
 *
 * - the first element in the list should contain in its "val" field the first
 *   letter of your first name; the second element the second letter, etc.;
 *
 * - the last element of the linked list should contain in its "val" field
 *   the last letter of your first name and its "link" field should be a null
 *   pointer
 *
 * - each element must be dynamically allocated using a malloc() call
 *
 * - if any call to malloc() fails, your function must return NULL and must also
 *   free any heap memory that has been allocated so far; that is, it must not
 *   leak memory when allocation fails
 *  
 *********************************************************************/

struct elt {
  char val;
  struct elt *link;
};

struct elt *name_list (void)
{
	size_t size_of_elt = sizeof(struct elt);
	struct elt *node = malloc(size_of_elt);
	if (node == NULL)
		return NULL;
	char name[7] = "Qinyun";
	node -> val = name[0];
	unsigned int i = 1;
	for (struct elt *t = node; i < strlen(name); ++i)
	{
		t -> link = malloc(size_of_elt);
		if (t == NULL)
		{
			for (struct elt *temp = node; temp != NULL; )
			{
				struct elt *tem = temp;
				temp = temp -> link;
				free(tem);
			}
			return NULL;
		}
		t = t -> link;
		t -> val = name[i];
	}
	return node;
}

/*********************************************************************
 *
 * convert()
 *
 * specification: depending on the value of "mode", print "value" as
 * octal, binary, or hexidecimal to stdout, followed by a newline
 * character
 *
 * extra requirement 1: output must be via putc()
 *
 * extra requirement 2: print nothing if "mode" is not one of OCT,
 * BIN, or HEX
 *
 * extra requirement 3: all leading/trailing zeros should be printed
 *
 * EXAMPLE: convert (HEX, 0xdeadbeef) should print
 * "00000000deadbeef\n" (including the newline character but not
 * including the quotes)
 *
 *********************************************************************/

enum format_t {
  OCT = 66, BIN, HEX
};

void convert (enum format_t mode, unsigned long value)
{
	char ans[65];
	for (int i = 0; i < 65; ++i)
		ans[i] = '0';
	int should_length = 0;
	switch (mode)
	{
			case OCT:
			for (int i = 0; value != 0; value >>= 3)
			{
				int number = value & 7;
				ans[i++] = '0' + number;
			}
			should_length = 20;
			break;
		case BIN:
			for (int i = 0; value != 0; value >>= 1)
				ans[i++] = (value & 1) + '0';
			should_length = 63;
			break;
		case HEX:
			for (int i = 0; value != 0; value >>= 4)
			{
				int number = value & 15;
				if (number < 10)
					ans[i++] = number + '0';
				else
					ans[i++] = (number - 10) + 'a';
			}
			should_length = 15;
			break;
		default: return;
	}
	for (int i = should_length; i >= 0; --i)
		putc(ans[i], stdout);
	putc('\n', stdout);
}

/*********************************************************************
 *
 * draw_me()
 *
 * this function creates a file called me.txt which contains an ASCII-art
 * picture of you (it does not need to be very big). the file must (pointlessly,
 * since it does not contain executable content) be marked as executable.
 * 
 * extra requirement 1: you may only call the function syscall() (type "man
 * syscall" to see what this does)
 *
 * extra requirement 2: you must ensure that every system call succeeds; if any
 * fails, you must clean up the system state (closing any open files, deleting
 * any files created in the file system, etc.) such that no trash is left
 * sitting around
 *
 * you might be wondering how to learn what system calls to use and what
 * arguments they expect. one way is to look at a web page like this one:
 * http://blog.rchapman.org/post/36801038863/linux-system-call-table-for-x86-64
 * another thing you can do is write some C code that uses standard I/O
 * functions to draw the picture and mark it as executable, compile the program
 * statically (e.g. "gcc foo.c -O -static -o foo"), and then disassemble it
 * ("objdump -d foo") and look at how the system calls are invoked, then do
 * the same thing manually using syscall()
 *
 *********************************************************************/

void draw_me (void)
{
	int create_file = syscall(85, "me.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (create_file == -1)
		return;

	int output_file = syscall(2, "me.txt", O_WRONLY);
	if (output_file == -1)
		return;

	char myself[2317] = "MMMMNNNNNNMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMDZ$ZZZZZZZZOOOOZNNMNNNNNNNMNN8MMMMNODMMMMMMMMMMMMMMMMMMMMM\nMNOI????IIIIIIIII?NNNNDN8DDNN8OZMMNDZO8MMMMNNMMMMIO88IMMMMMM\nMNOI????IIIIIIII77NNNNDN8D8NNOOOZMN8DD8NMMMMMMNMIZON$INMMMMM\nMNOI????IIIII$II77NDMNNNDN88NM$O$NDDNDONMMNNNNM88NDD$M8MMMMM\nMNO7???IIIII7Z8I8ZNDNNMZ$ND8DNN$$IMDNNODMNNNNDND88NDN8DMMMMM\nMNZ?+???IIII?ONNDONNNMZNOONDDDNN7I?DDNN8MMDDDNND8NDNDMMMMMMM\nMNZ?~~=+?III?ONNODNNN88DN7NNNNNDDNI?DNN8DNDDDNDDDNDDMMMMMMMM\nMNO?~~==?II??NNM8NNNN7DDDDIMNNNDDDDOO8NDDNDDNDDNDONMMMMNMMMM\nMNO?:====???8ND8DNNNONDDDDDONNNNDNNN8DMN8NDDDNNNDNMMMMNNMMMM\nMNOD$$7=~~~~DNNDDDNNOD8MNDDOZMNNDNDNNDM8DDDDDNNNDNNMNNDNNMMM\nMNZ8ZOZOZ$$MDDZDDDNDDDND$NDDZ8MNND8NNDDO8DMDDNNDNNMMNNNNMMMM\nMNZ?8888DD8DN$NDDNN8D$8ND8M8DDONDONNOOZDN8ZMDMNNDMMMNNNMMMMM\nMNO+$8888DDDNDDDDDNNN8OO8MDDDNM88ON888DD7M8D8MNDDNMMN8O8DNMM\nMNOI~D888DDDDDN8DDDDNNN8NDDDNMDDD88O88ZO8N88NDNDDNDZODZDDMMM\nMNOI==?88DDDDNDNNDND8ZNNDD8NNDZMODODN8DNNDDD8DNDDM8ODDZNMMMM\nMNOI+++=+NDD8DDNNNNND888D8NNDDNDDDNDND8DNNNN8NNDNNOODMMDDNMM\nMNOI+++++7ZD88ON8NDNNDNDDNN8DDDDN8+~+7DDNNDOONDNNNNND88NNMMM\nMNOI++++++=7D8NNDDDDDDMNDNDNDMDN8=:::~ODNN8ZNDNNNNNDNDMMMMMM\nMNOI=+++++++.NNNDODONNNDNNNNONNNO:~~~8DNNN8NNNDNNNNNMMMMMMMM\nMNOI=++==::IO7?N8NNNO8NNDDN8NNNDO7ZO8ND8NDDNNN8DNNNNMMMMMMMM\nMNOI++=O8ZO8O87$N$=~8N8DNNNNNNNZ8NDD8O8ZDNDNNNND8NNDMMMMMMMM\nMNOI=++++8OZOZO?O8O~~~DNNDDDDND8ZO8D8DDDDD8NNNNNDD88MMNMMMMM\nMNO7I+++++I$8OOZZDDD888$D8OOOZZZ88OOZ$$Z7MNNNNNNNDDNMMMMMMMM\nMNO7??++++++++7O88ZD88888$O$$8Z88DN7$7ZMMNNNNNNNNNNDMMMMMMMM\nMNOI++++++++I7777~8O$O78D888DD88NZ7777DDD88DDNNNNNNNMMMMMMMM\nMNO7??++??$7II777=$8O7$I$NOD8DDOZ$7777DDNNDDDDDNNNNNNNMMMMMM\nMNO7??????77III77+7ODID$7$Z7N8M$ON8Z$IDDDNNNDDDDDDNNNNMMMMMM\nMNO7?????I77777777$7888D$D7Z$$7ZD8DDDDDDNDNNNNNNNDDDNMMMMMMM\nMN87????I7777777777I?D8DN$DNN7ZOD8DDNDDDNNNNNNNNNNMMMMNMMMMM\nMM8$???77777777777II=Z8DDD8NNN8$MNDNNNNNNDDDNNNMNMMMMMMMMMMM\nMM8Z77777777777777I+==N8NDNDNNN8NNNNNNNNNNDDNDDNMMNNNNMMMMMM\nMM8Z7777777777777II=+$88DNDDDNNDNDDNNNNNNNNDDNNDDMMNNMMMMMMM\nMMDZ77$777777I777I7=88O8DDDNDNNNNDNDNNNNNNNNNDNMNNNNMMMMMMMM\nMMDZ7777777777777I7=D888NDNDDDNNNDNDNNNNNNNNDMMMNNNMMMNNMMMM\nMMN8OOOOOOOOOOOOOOO$NDDN8NNDNNNNMNNNMMMNMNMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM";

	//unsigned int write_success = write(output_file, myself, strlen(myself));
	unsigned int write_success = syscall(1, output_file, myself, strlen(myself));
	if (write_success != 2317)
	{
		int close_success = syscall(3, output_file);
		if (close_success < 0)
			return;
		syscall(87, "me.txt");
		return;
	}	
	int close_success = syscall(3, output_file);
	if (close_success < 0)
	{
		syscall(87, "me.txt");
		return;
	}
}
