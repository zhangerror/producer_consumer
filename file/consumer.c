#include "fctlTst.h"
static int num = 10;

int main(int argc, char* argv[]) {
	char buf[100], tmp[100];
	int fd, num, i, count;
	fd = open("VirFifo", O_RDWR);
	if(fd < 0) {
		perror("open error");
		exit(1);
	}
	if(argc == 2) num = atoi(argv[1]);
	if(num == 0) {
		printf("nothing to get\n");
		exit(0);
	}

	lock_set(fd, F_RDLCK);
	count = read(fd, buf, 100);
	lock_set(fd, F_UNLCK);
	
	if(count < num) {
		printf("insufficient resources: all = %d, need = %d", count, num);
		exit(1);
	}
	
	for(i=0; i<count; ++i) {
		tmp[i] = buf[i+num];
	}
	
	lock_set(fd, F_WRLCK);
	
	ftruncate(fd, 0);
	lseek(fd, 0L, SEEK_SET);
	count -= num;
	write(fd, tmp, count);
	//lseek(fd, 0L, SEEK_SET);
	
	lock_set(fd, F_UNLCK);
	
	for(i=0; i<num; ++i) {
		printf("%c", buf[i]);
	}
	printf("\n");

	close(fd);

	exit(0);
}
