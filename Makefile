ipv4grep: ipv4grep.c
	gcc -o ipv4grep ipv4grep.c

userinstall:
	cp -p ipv4grep    $(HOME)/bin/ipv4grep
	chmod u=rwx,go=rx $(HOME)/bin/ipv4grep
