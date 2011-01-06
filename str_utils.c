#include <system.h>
#include "config.h"

/*
 *	KPRINTF_BUFFER_SIZE used by kprintf which has a max line size of 255.
 *	We _have_ to use a fixed size buffer because it's used before malloc()
 *	becomes available.
*/
#define KPRINTF_BUFFER_SIZE 255

#define BASE_DECIMAL 10
#define BASE_HEX 16
#define BASE_OCT 8

/* kprintf / kbprintf interpretations:
 * %s = char*
 * %c = char (unsigned)
 * %i = unsigned integer
 * %x = hex
 * %o = octal
 * %% = % character
 * %X = unsigned long long (as hex)
 * %U = unsigned long long (as decimal)
 * obvious todo: %f for float (with formatting e.g. %.2f)
*/


/* these functions actually convert numbers to strings */
int ull_number_to_str(char *buffer, int max_size, unsigned long long number, int base);
int number_to_str(char *buffer, int max_size, int number, int base);

/* these handle the arguments to the functions above. Have a 1 to 1 mapping with char codes */
/*   %i   */ int int_to_str(char *buffer, int max_size, int number);
/*   %x   */ int int_to_hex_str(char *buffer, int max_size, int number);
/*   %o   */ int int_to_oct_str(char *buffer, int max_size, int number);
/*   %X   */ int unsigned_long_long_to_hex(char *buffer, int max_size, unsigned long long number);
/*   %U   */ int unsigned_long_long_to_str(char *buffer, int max_size, unsigned long long number);

void kprintf(char *format, ...)
{
	char buffer[KPRINTF_BUFFER_SIZE];

	int bpos = 0; /* position to write to in buffer */
	int fpos = 0; /* position of char to print in format string */
	char ch; /* current character being processed*/

	/*
	 * We have a variable number of paramters so we
	 * have to increment from the position of the format
	 * argument.
	*/
	int arg_offset = 1;

	/*
	 * Think this through Phill. &format = address of format on stack.
	 * &(format + 1) = address of argument after format on stack.
	 * void *p = &(format + arg_offset);
	 * kprintf("xxx %i %s", 32, "hello");
	 * memory would look like = [ 3, 32, 5, "xxx", 32, "hello" ]
	 * get to 32 via p = &(format + 1); (int)p (because the int is copied, not a pointer)
	 * get to hello via p = &(format + 2); (char*)p;
	 */

	void *arg;
	unsigned long long *llu;
	arg = (void*) (&format + arg_offset);
	llu = (unsigned long long*) *(&format + arg_offset);

	while (1)
	{
		ch = format[fpos++];
	
		if (ch == '\0')
			break;

		if (ch != '%')
			buffer[bpos++] = ch;
		else
		{
			ch = format[fpos++];
			if (ch == 's')
				bpos += strcpy(&buffer[bpos], KPRINTF_BUFFER_SIZE - bpos, (char*)arg);
			else if (ch == '%')
				buffer[bpos++] = '%';
			else if (ch == 'i')
				bpos += int_to_str(&buffer[bpos], KPRINTF_BUFFER_SIZE - bpos, *((int*)arg));
			else if (ch == 'x')
				bpos += int_to_hex_str(&buffer[bpos], KPRINTF_BUFFER_SIZE - bpos, *((int*)arg));
			else if (ch == 'o')
				bpos += int_to_oct_str(&buffer[bpos], KPRINTF_BUFFER_SIZE - bpos, *((int*)arg));
			else if (ch == 'X') {
				//arg is expected to be a pointer we need to further dereference.
				bpos += unsigned_long_long_to_hex(&buffer[bpos], KPRINTF_BUFFER_SIZE - bpos, *llu);
			} else if (ch == 'U') {
				bpos += unsigned_long_long_to_str(&buffer[bpos], KPRINTF_BUFFER_SIZE - bpos, *llu);
			} else
			{
				puts("invalid char ");
				putch(ch);
				puts(" passed to kprintf\n");
			}

			arg_offset++;
			arg = (void *)(&format + arg_offset);
			llu = (unsigned long long*) *(&format + arg_offset);
		}
	}

	buffer[bpos] = '\0';
	puts(buffer);
}

int kbprintf(char *buffer, int max_size, const char* format, ...)
{

	int bpos = 0; /* buffer position */
	int fpos = 0; /* format position */
	char ch; /* current character being processed*/

	/*
	 * We have a variable number of arguments we
	 * have to increment it from the position of the format
	 * argument. 
	*/
	int arg_offset = 1;
	void *arg;
	unsigned long long *llu;
	arg = *(&format + arg_offset);
	llu = (unsigned long long*) *(&format + arg_offset);

	while (1)
	{
		ch = format[fpos++]; // next char.
	
		if (ch == '\0')
			break;

		//encountered a special character.
		//increment to see how to interpret.
		if (ch != '%')
			*(buffer + bpos++) = ch;
		else
		{
			ch = format[fpos++];
			if (ch == 's')
				bpos += strcpy((buffer + bpos), max_size - bpos, arg);
			else if (ch == '%')
				*(char*)(buffer + bpos++) = '%';
			else if (ch == 'i')
				bpos += int_to_str((char*)buffer + bpos, max_size - bpos, (int)arg);
			else if (ch == 'x')
				bpos += int_to_hex_str((char*)buffer + bpos, max_size - bpos, (int)arg);
			else if (ch == 'o')
				bpos += int_to_oct_str((char*)buffer + bpos, max_size - bpos, (int)arg);
			else if (ch == 'X') {
				bpos += unsigned_long_long_to_hex((char*)buffer + bpos, max_size - bpos, *llu);
			} else if (ch == 'U') {
				bpos += unsigned_long_long_to_str((char*)buffer + bpos, max_size - bpos, *llu);
			} else
			{
				puts("invalid char ");
				putch(ch);
				puts(" passed to kprintf\n");
			}

			arg_offset++;
			arg = *(&format + arg_offset);
			llu = *(&format + arg_offset);
		}
	}

	*(buffer + bpos) = '\0';
	return bpos;
}

int int_to_str(char *buffer, int max_size, int number)
{
	return number_to_str(buffer, max_size, number, BASE_DECIMAL);
}

int int_to_hex_str(char *buffer, int max_size, int number)
{
	return number_to_str(buffer, max_size, number, BASE_HEX);
}

int int_to_oct_str(char *buffer, int max_size, int number)
{
	return number_to_str(buffer, max_size, number, BASE_OCT);
}

int unsigned_long_long_to_hex(char *buffer, int max_size, unsigned long long number)
{
	return ull_number_to_str(buffer, max_size, number, BASE_HEX);
}

int unsigned_long_long_to_str(char *buffer, int max_size, unsigned long long number) {
	return ull_number_to_str(buffer, max_size, number, BASE_DECIMAL);
}

int ull_number_to_str(char *buffer, int max_size, unsigned long long number, int base) {
	
	int bufpos = 0;

	unsigned int lo_byte = (unsigned int) number;
	unsigned int hi_byte = (unsigned int) (number >> 32);

	bufpos = number_to_str(buffer, max_size, lo_byte, base);
	bufpos += number_to_str(buffer + bufpos, max_size, hi_byte, base);

	return bufpos;

}

#define NUMERIC_BUFF_SIZE (11 * (ADDRESS_SIZE / 32))

int number_to_str(char *buffer, int max_size, int number, int base)
{
	char *char_map = "0123456789ABCDEF";

	int remain = 0;
	char buff_stack[NUMERIC_BUFF_SIZE];
	int stk_pnt = 0;
	int bpos = 0;

	/* with this method of parsing, the digits come out backwards */
	do 
	{
		if (stk_pnt > NUMERIC_BUFF_SIZE)
		{
			puts("Number has too many digits to be printed. Increasse NUMBERIC_BUFF_SIZE\n");
			return 0;
		}

		remain = number % base;
		number = number / base;
		buff_stack[stk_pnt++] = char_map[remain];
	} while (number > 0);

	/* before writing...ensure we have enough room */
	if (stk_pnt > max_size)
	{
		//error. do something?
		puts("number_to_str passed number with too many digits to go into buffer\n");
		//printf("error. stk_pnt > max_size (%d > %d)\n", stk_pnt, max_size);
		return 0;
	}

	/* reorder */
	while (stk_pnt > 0)
		buffer[bpos++] = buff_stack[--stk_pnt];
	
	return bpos;
}

/* return the number of chars appended */

int strcpy(char *buffer, int max_size, char *append)
{

	int i = 0, j;

	for (j = 0; *(append + j) != '\0'; j++)
		*(buffer + i++) = *(append + j);

	return j;
}

int strcat(char *buffer, int max_size, char *append)
{

	int i, j;

	for (i = 0; *(buffer + i) != '\0'; i++)
		/*no body */;

	for (j = 0; *(append + j) != '\0'; j++)
		*(buffer + i++) = *(append + j);

	return j;
}

size_t strlen(const char *str)
{
    size_t retval;
    for (retval = 0; *str != '\0'; str++)
		retval++;
    
	return retval;
}

