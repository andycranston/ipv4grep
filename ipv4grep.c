static char *version = "@(!--#) @(#) ipv4grep.c, sversion 0.1.0, fversion 011, 21-june-2023";

/*
 *  ipv4grep.c
 *
 *  grep the first part of a file for an IP address
 *
 */

/**********************************************************************/

/*
 *  Links:
 *
 *  Notes:
 *
 */

/**********************************************************************/

/*
 *  includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**********************************************************************/

/*
 *  defines
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULT_BUF_SIZE 1000000

#define MIN_IPV4_ADDRESS_LENGTH 7
#define MAX_IPV4_ADDRESS_LENGTH 15

#define MAX_LINE_LENGTH 1024

/**********************************************************************/

/*
 *  global variables
 */

char    *progname;

/**********************************************************************/

int stringbegins(s1, s2)
	char	*s1;
	char	*s2;
{
	int	lens1;
	int	lens2;

	lens1 = strlen(s1);
	lens2 = strlen(s2);

	if (lens1 < lens2) {
		return FALSE;
	}

	if (strncmp(s1, s2, lens2) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/**********************************************************************/

int octetlength(o)
	char	*o;
{
	int	len;

	len = 0;

	while ((*o != '.') && (*o != '\0')) {
		len++;

		if (len > 4) {
			break;
		}

		o++;
	}

	return len;
}

/**********************************************************************/

int validipv4address(ipv4address)
	char	*ipv4address;
{
	int	lenipv4address;
	int	numdots;
	int	numdigits;
	int	i;
	char	c;
	char	*octetarray[4];
	int	octetindex;
	int	octetvalue;
	int	lenoctet;

	lenipv4address = strlen(ipv4address);

	if ((lenipv4address < MIN_IPV4_ADDRESS_LENGTH) || (lenipv4address > MAX_IPV4_ADDRESS_LENGTH)) {
		return FALSE;
	}

	numdots = 0;
	numdigits = 0;

	for (i = 0; i < lenipv4address; i++) {
		c = ipv4address[i];

		if (c == '.') {
			numdots++;
		} else if (isdigit(c)) {
			numdigits++;
		} else {
			return FALSE;
		}
	}

	if (numdots != 3) {
		return FALSE;
	}

	if (numdigits < 4) {
		return FALSE;
	}

	if (ipv4address[0] == '.') {
		return FALSE;
	}

	if (ipv4address[lenipv4address - 1] == '.') {
		return FALSE;
	}

#ifdef DEBUG
	printf("Finding octet positions\n");
#endif

	octetindex = 0;

	octetarray[octetindex] = ipv4address;

	for (i = 0; i < lenipv4address; i++) {
		c = ipv4address[i];

		if (c == '.') {
			octetindex++;

			if (octetindex == 4) {
				return FALSE;
			}

			octetarray[octetindex] = ipv4address + i + 1;
		}
	}

	if (octetindex != 3) {
		return FALSE;
	}

#ifdef DEBUG
	printf("Checking octets\n");
#endif

	for (i = 0; i < 4; i++) {
#ifdef DEBUG
		printf("[%s] %ld\n", octetarray[i], strlen(octetarray[i]));
#endif

		lenoctet = octetlength(octetarray[i]);

#ifdef DEBUG
		printf("lenoctet=%d\n", lenoctet);
#endif

		if ((lenoctet < 1) || (lenoctet > 3)) {
			return FALSE;
		}

		if (*(octetarray[i]) == '.') {
			return FALSE;
		}

		if (lenoctet > 1) {
			if (*(octetarray[i]) == '0') {
				return FALSE;
			}
		}

		octetvalue = atoi(octetarray[i]);

		if ((octetvalue < 0) || (octetvalue > 255)) {
			return FALSE;
		}
	}

	return TRUE;
}

/**********************************************************************/

int match(s1, s2, len)
	char	*s1;
	char	*s2;
	int	len;
{
	int	i;

	for (i = 0; i < len; i++) {
		if (s1[i] != s2[i]) {
			return FALSE;
		}
	}

	return TRUE;
}

/**********************************************************************/

int ipv4grep(buf, bufsize, filename, ipv4address)
	char	*buf;
	int	bufsize;
	char	*filename;
	char	*ipv4address;
{
	int	lenipv4address;
	int	fd;
	ssize_t	bytesread;
	int	found;
	int	i;

	lenipv4address = strlen(ipv4address);

	fd = open(filename, O_RDONLY);

	if (fd == -1) {
		fprintf(stderr, "%s: cannot open file \"%s\" for reading\n", progname, filename);
		return 2;
	}

	bytesread = read(fd, buf + sizeof(char), (size_t)bufsize);

        if (bytesread < 0) {
		fprintf(stderr, "%s: read error on file \"%s\"\n", progname, filename);

		if (close(fd) != 0) {
			fprintf(stderr, "%s: error closing file after read error\"%s\"\n", progname, filename);
			return 2;
		}

		return 2;
	}

	if (close(fd) != 0) {
		fprintf(stderr, "%s: error closing file \"%s\"\n", progname, filename);
		return 2;
	}

	buf[0] = '#';

	buf[bytesread + 1] = '#';

	found = FALSE;

	for (i = 1; i <= bytesread; i++) {
		if (! match(buf + i, ipv4address, lenipv4address)) {
			continue;
		}

		if (isdigit(buf[i - 1])) {
			continue;
		}

		if (isdigit(buf[i + lenipv4address])) {
			continue;
		}

		found = TRUE;
		break;
	}

	if (found) {
		printf("%s\n", filename);
		return 0;
	} else {
		return 1;
	}
}

/**********************************************************************/

char *nullfgets(line, maxlength, fhandle)
	char	*line;
	int	maxlength;
	FILE	*fhandle;
{
	int	currentlength;
	int	c;

	currentlength = 0;

	while (currentlength < (maxlength - 1)) {
		c = getc(fhandle);

		if (c == EOF) {
			break;
		}

		line[currentlength] = c;

		currentlength++;

		if (c == '\0') {
			break;
		}
	}

	if (c == '\0') {
		return line;
	}

	if ((c == EOF) && (currentlength == 0)) {
		return NULL;
	}

	if ((c == EOF) && (currentlength > 0)) {
		line[currentlength] = '\0';
		return line;
	}

	/* is we get here the buffer is full of data except for the last character */
	/* is we peek ahead it might be \0 or EOF which means we can terminate the string successfully using all the buffer space */
	/* otherwise put the character back on the stream with unget and we have a line overflow condition but at least we wont lose any data */

	c = getc(fhandle);

	if ((c == '\0') || (c == EOF)) {
		line[currentlength] = '\0';
		return line;
	} else {
		ungetc(c, fhandle);
		line[currentlength] = '\0';
		return line;
	}
}

/**********************************************************************/

/*
 *  Main
 */

/* function */
int main(argc, argv)
	int	argc;
	char	*argv[];
{
	/* local variables */
	int	i;
	char	*arg;
	char	*argvalue;
	int	bufsize;
	char	*ipv4address;
	char	*filename;
	int	ipv4checkflag;
	char	*buf;
	int	retcode;
	int	finalretcode;
	char	line[MAX_LINE_LENGTH];

	progname = argv[0];

        if (((argc - 1) % 2) != 0) {
		fprintf(stderr, "%s: expecting an even number of command line arguments\n", progname);
		exit(2);
	}

	bufsize = DEFAULT_BUF_SIZE;
	ipv4address = NULL;
	filename = NULL;
	ipv4checkflag = TRUE;

	for (i = 1; i < argc; i = i + 2) {
		arg = argv[i];
		argvalue = argv[i+1];

		if (strcmp(arg, "-s") == 0) {
			bufsize = atoi(argvalue);

			if (bufsize <= 0) {
				fprintf(stderr, "%s: buffer size of \"%s\" is invalid\n", progname, argvalue);
				exit(2);
			}
		} else if (strcmp(arg, "-i") == 0) {
			ipv4address = argvalue;

			if (strlen(ipv4address) > MAX_IPV4_ADDRESS_LENGTH) {
				fprintf(stderr, "%s: IPv4 address \"%s\" is too long\n", progname, ipv4address);
				exit(2);
			}
		} else if (strcmp(arg, "-f") == 0) {
			filename = argvalue;
		} else if (strcmp(arg, "-c") == 0) {
			if (strcmp(argvalue, "y") == 0) {
				ipv4checkflag = TRUE;
			} else if (strcmp(argvalue, "n") == 0) {
				ipv4checkflag = FALSE;
			} else {
				fprintf(stderr, "%s: IPv4 check command line argument \"-c\" must have a value of \"y\" or \"n\"\n", progname);
				exit(2);
			}
		} else {
			fprintf(stderr, "%s: invalid command line option \"%s\"\n", progname, arg);
			exit(2);
		}
	}

	if (ipv4address == NULL) {
		fprintf(stderr, "%s: must specify an IPv4 address on the command line with the -i option\n", progname);
		exit(2);
	}

	if (ipv4checkflag) {
		if (! validipv4address(ipv4address)) {
			fprintf(stderr, "%s: address \"%s\" is invalid\n", progname, ipv4address);
			exit(2);
		}
	}

#ifdef DEBUG
	printf("BUFSIZE=%d\n", bufsize);
	printf("IPv4 ADDRESS=[%s]\n", ipv4address);
	printf("FILENAME=[%s]\n", filename);
#endif

	buf = malloc(sizeof(char) + bufsize + sizeof(char));

	if (buf == NULL) {
		fprintf(stderr, "%s: unable to allocate %d bytes of memory for buffer\n", progname, bufsize);
		exit(2);
	}

	if (filename != NULL) {
		finalretcode = ipv4grep(buf, bufsize, filename, ipv4address);
	} else {
		finalretcode = 0;

		while (nullfgets(line, MAX_LINE_LENGTH, stdin) != NULL) {
/*
			if (strlen(line) > 0) {
				if (line[strlen(line) - 1] == '\n') {
					line[strlen(line) - 1] = '\0';
				}
			}
*/

			if (stringbegins(line, "/sys/")) {
				continue;
			}

			if (stringbegins(line, "/proc/")) {
				continue;
			}

			retcode = ipv4grep(buf, bufsize, line, ipv4address);

			if (retcode != 0) {
				if (finalretcode == 0) {
					finalretcode = retcode;
				}
			}
		}
	}

	return finalretcode;
}

/* end of file */
