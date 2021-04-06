suid-bomb: main.c
	gcc -o suid-bomb main.c

container: suid-bomb
	podman build . -t suid-bomb
