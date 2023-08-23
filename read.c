#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>

#define BLOCK_SIZE 4096
#define FILENAME "write.c"

#define SEM_1 "sem1"
#define SEM_2 "sem2"

static int get_block(char *filename, int size){
	key_t key;
	key = ftok(filename, 0);
	if(key == -1){
		return -1;
	}
	return shmget(key,size, 0644 | IPC_CREAT);
}

char * attach_block(char *filename, int size){
	int block_id = get_block(filename, size);
	char *result;
	result = shmat(block_id, NULL, 0);
	if(result == "-1"){
		return NULL;
	}
	return result;
}

bool detach_block(char *block){
	return (shmdt(block) != -1);
}

bool destroy_block(char *filename){
	int block_id = get_block(filename, 0);
	if(block_id == -1){
		return NULL;
	}
	return (shmctl(block_id, IPC_RMID, NULL) != -1);
}

int main(void){
	char *block = attach_block(FILENAME, BLOCK_SIZE);
	if(block == NULL){
		printf("Error! Unable to get shm block!");
		return -1;
	}
	sem_t *sem_1 = sem_open(SEM_1, 0);
	sem_t *sem_2 = sem_open(SEM_2, 1);
	while(true){
		sem_wait(sem_1);
		if(strcmp(block, "kraj\n") == 0){
			sem_post(sem_2);
			break;
		}
		printf("Poruka:\n%s", block);
		sem_post(sem_2);
	}
	sem_close(sem_2);
	sem_close(sem_1);
	detach_block(block);
	return 0;
}
