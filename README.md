# Search for IPv4 addresses in one or more files

The `ipv4grep` command searches a file (or files) for an IPv4 address. It has
logic to make sure an IPv4 address such as:

```
1.2.3.4
```

does not give a false positive match to a file containing:

```
11.2.3.44
```

The `ipv4grep` command also saves time when dealing with large files (e.g. database files) by
only searching the first 1,000,000 bytes of a file over 1,000,000 bytes in size.

While this behaviour may miss some files which do contain the IPv4 address the assumption is that
the `ipv4grep` command is being used to locate an IPv4 address in much smaller configuration files.

# Source code and compiling

The `ipv4grep` command source code is in a single file called:

```
ipv4grep.c
```

This can be compiled with the gcc C compiler:

```
gcc -o ipv4grep ipv4grep.c
```

and the resulting `ipv4grep` executable copied to a directory in the PATH environment variable.

# Searching a single file

Use a command line similar to:

```
ipv4grep -f filename -i 1.2.3.4
```

This will search the file `filename` for the IPv4 address `1.2.3.4` - if that IPv4 address is present at least once in the file the name of
the file is printed on stdout and a return code of 0 is given. If the IPv4 address is not found then no output is printed and
a return code of 1 is given.

Remember that only the first 1,000,000 bytes of a file is searched bu the `ipv4grep` command. IPv4 addresses are usually in files much smaller
than 1,000,000 bytes such as configuration files.

If more bytes need to be searched the `-s` command line option can be used. For example to search the first 5,000,000 bytes use:

```
ipv4grep -s 5000000 -f filename -i 1.2.3.4
```

# Searching multiple files

To search multiple files leave out the `-f` command line argument and filename and, instead, send the `ipv4grep`
command a list of filenames on standard input. For example:

```
find /etc -type f -print0 | ipv4grep -i 1.2.3.4
```

Note the use of the `-print0` command line argument with the find command. This uses a null character (ASCII code 0) to separate the
filenames. This is because when the `ipv4grep` command reads filenames from standard input it uses the null character as
the delimiter character.

# Linux systems and /sys and /proc

When reading file names from stdin any file names which begin:

```
/sys/
```

or:

```
/proc/
```

are ignored by the `ipv4grep` command. This is because on a Linux system the content of the files
under these directories is dynamic. Also trying to read from some of
these files will cause the `ipv4grep` command to hang/block on the read operation.

If you want files that begin `/sys/` and `/proc/` to be searched then delete these lines:

```
if (stringbegins(line, "/sys/")) {
        continue;
}

if (stringbegins(line, "/proc/")) {
        continue;
}
```

from the `ipv4grep.c` source code file before compiling it.

I might add a command line switch to toggle this.

# IPv4 address checking

The IPv4 address supplied to the `-i` command line argument is validated to make sure it is correctly formatted. For example:

```
0.0.0.0
1.2.3.4
128.7.56.201
255.255.255.255
```

are all valid IPv4 addresses but:

```
01.02.03.04
001.002.003.004
```

are not as the octets that make up an IPv4 address are not allowed to have leading zeroes.

If this is to restrictive the check for a valid IPv4 address can be skipped by using the `-c` command
line option with an argument of 'n' as follows:

```
ipv4grep -f filename -i 001.002.003.004 -c n
```

# Known issues

The matching code could probably be faster.

----------------
End of README.md
