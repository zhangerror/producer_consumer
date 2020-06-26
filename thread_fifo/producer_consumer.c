#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <semaphore.h>

#define N 10

pthread_mutex_t mutex;		// 互斥信号量
sem_t avail;				// 有界缓冲区中的空单元数
sem_t full;					// 有界缓冲区中的非空单元数

void *producer_opt(void *arg) {
	char buf[PIPE_BUF];	// 写缓冲
	
	while(1) {
		printf("producer write : ");
		fflush(stdout);
		sleep(1);
		/* 读取终端输入 */
		fgets(buf, sizeof(buf), stdin);
		/* 申请信号量 */
		pthread_mutex_trylock(&mutex);
		sem_wait(&avail);
		/* 将终端输入写入管道 */
		if((write((int)arg, buf, 1)) < 0) { 
			/* 写入失败 */
			/* 释放信号量 */
			sem_post(&avail);
			pthread_mutex_unlock(&mutex);
			perror("write");
		}
		/* 释放信号量 */
		sem_post(&full);
		pthread_mutex_unlock(&mutex);
	}
	
	pthread_exit("producer finished");
}

void *consumer_opt(void *arg) {
	int len = 0;			// 实际读取长度
	char buf[PIPE_BUF];	// 读缓冲
	
	/* 读取管道中的数据并打印到终端 */
	while(1) {	
		sleep(2);
		/* 申请信号量 */
		pthread_mutex_trylock(&mutex);
		sem_wait(&full);
		if((len = (read((int)arg, buf, 1))) > 0) {
			/* 释放信号量 */
			sem_post(&avail);
			pthread_mutex_unlock(&mutex);
			printf("\nconsumer read : %d, %c\n", len, buf[0]);
		}
		else {
			pthread_mutex_unlock(&mutex);
		}
	}
	
	pthread_exit("consumer finished");
}

int main()
{
	int res;
	pthread_t consumer, producer;		// 线程标识
	void *consumer_result, *producer_result;
	int fd;					// 管道描述符
	
	/* 初始化信号量 */
	pthread_mutex_init(&mutex, NULL);
	if(sem_init(&avail, 0, N)) {
		perror("semaphore avail failed");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&full, 0, 0)) {
		perror("semaphore full failed");
		exit(EXIT_FAILURE);
	}

	/* 打开管道 */
	if((fd = open("fifo_pc", O_RDWR | O_APPEND)) < 0) {
		/* 打开失败 */
		perror("open");
		exit(EXIT_FAILURE);
	}
	
	/* 创建线程 */
	res = pthread_create(&producer, NULL, producer_opt, (void *)fd);
	if (res != 0) {
		perror("producer creation failed");
		exit(EXIT_FAILURE);
	}
	printf("\nWaiting for producer to finish...\n");	
	
	res = pthread_create(&consumer, NULL, consumer_opt, (void *)fd);
	if (res != 0) {
		perror("consumer creation failed");
		exit(EXIT_FAILURE);
	}
	printf("\nWaiting for consumer to finish...\n");
	
	/* 合并线程 */
	res=pthread_join(producer,&producer_result);
	if (res != 0) {
		perror("producer join failed");
		exit(EXIT_FAILURE);
	}
	res=pthread_join(consumer,&consumer_result);
	if (res != 0) {
		perror("consumer join failed");
		exit(EXIT_FAILURE);
	}
	
	/* 显示线程返回值 */
	printf("producer joined : %s\n", (char*)producer_result);
	printf("consumer joined : %s\n", (char*)consumer_result);
	/* 销毁信号量 */
	pthread_mutex_destroy(&mutex);
	sem_destroy(&avail);
	sem_destroy(&full);
	/* 关闭管道 */
	close(fd);
	
	exit(EXIT_SUCCESS);
}