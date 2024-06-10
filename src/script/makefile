all:
	gcc -g -o kallsyms kallsyms.c
	nm -n ../Build/kernel.bin | ./kallsyms > kallsyms.S
	gcc -c kallsyms.S

clean:
	rm -rf kallsyms kallsyms.o kallsyms.S