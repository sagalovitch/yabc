#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <readline/readline.h>
#include <readline/history.h>

/** Define type for color */
typedef char *color_t;


#define red   "\033[0;31m"        /* 0 -> normal ;  31 -> red */
#define cyan  "\033[1;36m"        /* 1 -> bold ;  36 -> cyan */
#define green "\033[4;32m"        /* 4 -> underline ;  32 -> green */
#define blue  "\033[9;34m"        /* 9 -> strike ;  34 -> blue */

#define black  "\033[0;30m"
#define brown  "\033[0;33m"
#define magenta  "\033[0;35m"
#define gray  "\033[0;37m"

#define none   "\033[0m"        /* to flush the previous property */


#define _DUMP_HEADER_COLOR	red
#define _DUMP_COLOR		brown
#define _DUMP_HEX_COLOR		magenta
#define _DUMP_BITS_RANGE_COLOR	red
#define _HELP_COLOR		cyan
#define _ERROR_COLOR		cyan


char msg_help []            =   "Q        - Quit\n"
                                "RX[.Y]   - Display bits range [X-Y] value (X and Y are decimal)\n"
                                "X        - Set new value to work\n"
                                " e.g.\n"
                                "        0xBC6789FF  - Set new value to dump (hex. value)\n"
                                "        123         - Set new value to work (dec. value)\n"
                                "        R4.13       - Display value for bits range 4 - 13 \n"
                                "                      (dec. 0-based)\n"
                                "\n"
                                "\n"
                                "  Note:   If input line is empty than program dump the current value\n";

char msg_err_unrecognized []= "ERROR: Invalid input: %s. Enter 'h' for help\n";

void color_print(color_t fg, const char * msg, ...)
{
	va_list arglist;
	static char print_buffer[4096];

	va_start (arglist, msg);
	int retval = vsprintf(print_buffer, msg, arglist);
	va_end(arglist);
	printf(fg);
	printf(print_buffer);
	printf(none);
}


static void display_binary_dump32_internal(uint32_t value, char *header)
{
	int nIndex;


	color_print(_DUMP_HEADER_COLOR, header);
                                            // Print header

	color_print(_DUMP_COLOR, "  ");                     // Print space


	for (nIndex = (CHAR_BIT * sizeof(uint32_t) - 1); nIndex >= 0; nIndex--)
	{
		unsigned long  Mask = (1 << nIndex);// Create bit- mask

		color_print(_DUMP_COLOR, "%d ", (Mask & value) ? 1 : 0);

		if (!(nIndex % 8) && nIndex)
		color_print(_DUMP_COLOR, "- ");                 // Display separator between octet

	}

	color_print(none, "\n");


	color_print(_DUMP_HEX_COLOR, "         %02X                %02X                %02X                %02X\n",
			((value >> 24) & 0xFF),
			((value >> 16) & 0xFF),
			((value >> 8) & 0xFF),
			((value) & 0xFF));

}

void display_binary_dump32(uint32_t value)
{
	display_binary_dump32_internal(value, " 31                23                15                 7             0\n");
}

void display_binary_dump64(uint64_t value)
{
	int nIndex;                             // Index in loop

	uint32_t low_part  = (uint32_t) (value & 0xffffffff);
	uint32_t high_part = (uint32_t) ((value & 0xffffffff00000000) >> 32);

	display_binary_dump32(high_part);
	display_binary_dump32_internal(high_part, " 63                55                47                39            32\n");
	display_binary_dump32_internal(low_part,  " 31                23                15                 7             0\n");
}

/**
 *   
 *   Check if this is the valid bit (0-32 range)
 *
 *   @param   pos  - \c [in]  Position to check
 *
 *   @return  1 if this is the valid bit (0-31 range)\n
 *            0 - otherwise
 *
*/
int is_valid_bit_index32(int pos)
{
	if ((pos >= 0 && pos < CHAR_BIT * sizeof(uint32_t)))
		return 1;
	else
		return 0;
}


/**
 * Display value for bits range
 *
 * @param   value        - \c [in]  Value from which take bits-range
 * @param   first        - \c [in]  First bit (0-based)
 * @param   last         - \c [in]  Last bit in range (0-based)
 *
 * @note Work only with 32-bit numbers
 *
*/
void display_bits_range_value(uint32_t value, int first, int last)
{
        /* Calculate number of bits left  till end of 'unsigned long' 
	 * from LastBit */
	uint64_t bits_till_end = (uint64_t)CHAR_BIT * sizeof(uint32_t) -
						((uint64_t)last + 1);

	/* Build bits range value */

        /** Build mask */

	value = value << bits_till_end;
	value = value >> (bits_till_end + first);

	/* Display bits range value */

	if (first != last) {
		color_print(_DUMP_BITS_RANGE_COLOR,
				"Bits [%02d:%02d] = %04Xh (Dec. %lu)\n",
				first, last, value, value);
	} else {
		color_print(_DUMP_BITS_RANGE_COLOR,
				"Bit [%02d] = %lld\n", first, value);
	}
}
/**
 * Try to get bits range in format RX.Y or RX. X and Y is dec.0-based values
 *
 * @param   str   - \c [in] String to parse 
 * @param   first - \c [out] First bit (0-based)  
 * @param   last  - \c [out] Last bit
 *
 * @return  1 - if operation was successful
 *	    0 - otherwise
 *
*/
int get_bits_range(char * str, uint32_t * first , uint32_t * last)
{
	int result = 0; /* Assume operation failed */
	char *endptr;
	unsigned long int value = strtoul (str, &endptr, 0);


	*first = *last = 0;

	if (endptr[0] == '\0') {

		*first = (uint32_t) value;

		if (is_valid_bit_index32(*first)) {
			result = 1;
			*last = *first;

		}
	}

	if (!result) {

	        char seps[] = ".\n";

		char *bit_position = strtok(str, seps);

		if (bit_position) {

			*first = strtoul (bit_position, &endptr, 0);

			bit_position = strtok(NULL, seps);

	                if (bit_position) {
				*last = strtoul (bit_position, &endptr, 0);

				if (is_valid_bit_index32(*first) &&
				   is_valid_bit_index32(*last)   &&
				   *last >= *first)
					result = 1;
			}
		}
	}

	return result;
}

/**
 * Main program loop
 *
 *
*/
void run_loop(void)
{
	uint32_t value 	= 0;
	char *line 	= NULL;
	char *s 	= NULL;
	int   valid_cmd = 0;

	 /* Loop reading and executing lines until the user quits. */
	for (; ;) {
		valid_cmd = 0;

		line = readline ("# : ");

		if (!line)
			break;

		s = line; //stripwhite (line);

		if (s) {
			if (*s) {
				if (s[0] == 'q' || s[0] == 'Q') {
						/* We want to quit */
						break;
				} else if (s[0] == 'h' || s[0] == 'H') {
					color_print(_HELP_COLOR, msg_help);
					continue;
				}
				else if (s[0] == 'R' || s[0] == 'r') {

					uint32_t first, last;

					valid_cmd = get_bits_range(&s[1],
								&first,
								&last);
					if (valid_cmd) {
						display_bits_range_value(value,
								first,
								last);
					}
				} else {
					/*  Assume that this is the new number */

					unsigned long temp_value;
					char *endptr;

					temp_value = strtoul (s, &endptr, 0);

					if (endptr[0] == '\0') {
						valid_cmd = 1;
						value = temp_value;
						display_binary_dump32(value);
					}
				}

				if (valid_cmd) {
					add_history (s);
				} else {
					color_print(_ERROR_COLOR,
							msg_err_unrecognized,
							line);
				}

				free (line);
			}
		}
	}
}

int main(void)
{
	printf("%s\n", PACKAGE_STRING);
	printf("Please send bug report to %s\n", PACKAGE_BUGREPORT);
	printf("Use 'q' for and 'h' for help\n");

	run_loop();
	return 0;
}

