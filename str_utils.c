#include <system.h>

/* size of buffer used by kprintf only */
/*
 * If this size constraint becomes a problem
 * the code can use "putch" directly to avoid
 * the need for the buffer.
*/
#define BUFFER_SIZE 255

#define BASE_DECIMAL 10
#define BASE_HEX 16
#define BASE_OCT 8

/* kprintf / kbprintf interpretations:
 * %s = char*
 * %c = char
 * %i = integer
 * %x = hex
 * %o = octal
 * %% = % character
 * obvious todo: %f for float (with formatting e.g. %.2f)
*/


int number_to_str(char *buffer, int max_size, int number, int base);
int int_to_str(char *buffer, int max_size, int number);
int int_to_hex_str(char *buffer, int max_size, int number);
int int_to_oct_str(char *buffer, int max_size, int number);

void kprintf(char *format, ...)
{
	char buffer[BUFFER_SIZE];

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
	arg = (void*) (&format + arg_offset);

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
				bpos += strcpy(&buffer[bpos], BUFFER_SIZE - bpos, (char*)arg);
			else if (ch == '%')
				buffer[bpos++] = '%';
			else if (ch == 'i')
				bpos += int_to_str(&buffer[bpos], BUFFER_SIZE - bpos, *((int*)arg));
			else if (ch == 'x')
				bpos += int_to_hex_str(&buffer[bpos], BUFFER_SIZE - bpos, *((int*)arg));
			else if (ch == 'o')
				bpos += int_to_oct_str(&buffer[bpos], BUFFER_SIZE - bpos, *((int*)arg));
			else
			{
				puts("invalid char ");
				putch(ch);
				puts(" passed to kprintf\n");
			}

			arg_offset++;
			arg = (void *)(&format + arg_offset);
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
	arg = *(&format + arg_offset);

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
			else
			{
				puts("invalid char ");
				putch(ch);
				puts(" passed to kprintf\n");
			}

			arg_offset++;
			arg = *(&format + arg_offset);
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

#define NUMERIC_BUFF_SIZE 32

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

