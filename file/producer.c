#include "fctlTst.h"

#define CH 0

static int cycle = 1;
static int num = 10;
static int count = 0;

int main(int argc, char* argv[]) {
	int fd, wr;
	char buf[100];
	char ch[] = "a", nr[] = "0";
	wr = 0;
	fd = open("VirFifo", O_RDWR | O_CREAT, 0666);
	if(fd < 0) {
		perror("open error: VirFifo\n");
		exit(1);
	}
	if(argc == 2) cycle = atoi(argv[1]);
	else if(argc == 3) { cycle = atoi(argv[1]);	num = atoi(argv[2]);	}

	while(1) {
		lock_set(fd, F_RDLCK);
		lseek(fd, 0L, SEEK_SET);
		count = read(fd, buf, strlen(buf));
		lock_set(fd, F_UNLCK);
		
		if(count < num) {
		printf("count = %d, num = %d\n", count, num);
			lock_set(fd, F_WRLCK);
			
			lseek(fd, 0L, SEEK_END);
			if(wr&1 != CH) { write(fd, ch, strlen(ch));	++*ch; }
			else { write(fd, nr, strlen(nr));	++*nr; }
			lseek(fd, 0L, SEEK_SET);
			
			lock_set(fd, F_UNLCK);

			++wr;
		}
		sleep(cycle);
	}

	close(fd);
	exit(0);
}
