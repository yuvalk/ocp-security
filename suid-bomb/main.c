#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int read_secret() {
	FILE *fp;
	int nRC = 0;
	char line[1000];
	char ch;

	if ((fp = fopen ("secret.txt","r")) == NULL) {
		printf ("Error! opening file\n");
		nRC = 1;
	} else {
		while((ch = fgetc(fp)) != EOF)
			printf("%c", ch);

		fclose(fp);
	}

	return nRC;
}

int main(){
	printf("start\n");
	
	uid_t uid = 0; // root
	int nRC = 0;

	printf("try to read secert\n");
	read_secret();

	printf("setuid: %d\n", uid);
	nRC = setuid(uid);
	if (nRC != 0) {
		printf ("setuid=%d, errno=%d\n", nRC, errno);
		exit (1);
	}
	
	printf("try to read secert with uid 0\n");
	read_secret();

	printf("exit\n");
}
