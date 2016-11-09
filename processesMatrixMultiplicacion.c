#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void initSeed(void);
void populateMatrix(int*, int);
void printMatrix(int*, int);
void multiply(int*, int*, int*, int, int, int);

int POPULATE_RANDOM = 1;
int NO_POPULATE_RANDOM = 0;
int MIN_VALUE = 1;
int MAX_VALUE = 4;

int main(int argc, char* argv[]) {

	int nSize = 0;
	int numberOfProcesses = 0;
	key_t key;
	int shmid;
	int* startSharedMemory;
	int* startMatrixA;
	int* endMatrixA;
	int* startMatrixB;
	int* endMatrixB;
	int* startMatrixC;
	int* endMatrixC;
	int sizeByProcess;
	pid_t pid;

	if(argc != 3) {
		printf("Unexpected number of arguments.\n");
		printf("Usage: processesMatrixMultiplication n p, where n is the size of the matrix and p is the number of processes.\n");
		exit(EXIT_FAILURE);
	}

	int data = sscanf(argv[1],"%d", &nSize);
	
	if(data != 1) {
		printf("Error: the size of the matrix must be an integer.\n");
		exit(EXIT_FAILURE);
	}

	data = sscanf(argv[2],"%d", &numberOfProcesses);
	
	if(data != 1) {
		printf("Error: the number of processes must be an integer.\n");
		exit(EXIT_FAILURE);
	}

	if(nSize%numberOfProcesses!=0) {
		printf("Error: the matrix size must be a multiple of the number of processes.\n");
		exit(EXIT_FAILURE);
	}

	key = ftok(".", 'R');

	if(key == -1) {
		printf("Error: the identifier of the file could not be created.\n");
		exit(EXIT_FAILURE);		
	}

	shmid = shmget(key, sizeof(int)*nSize*nSize*3, 0644 | IPC_CREAT);

	if(shmid == -1) {
		int err = errno;
		printf("Error: the shared memory could not be created. Error %d.\n", err);
		exit(EXIT_FAILURE);	
	}

	startSharedMemory = shmat(shmid, (void*)0, 0);

	if(startSharedMemory == (int*)(-1)) {
		printf("Error: the shared memory could not be accessed.\n");
		exit(EXIT_FAILURE);	
	}

	//printf("Address shared memory: %p\n", startSharedMemory);

	//StartA = 1 size = 1
	//EndA = 1 + size*size - 1 = 1 + 1 - 1 = 1
	//StartB = 1 + 1 = 2
	//EndB = 2 + size*size - 1 = 2 + 1 - 1 = 2
	startMatrixA = startSharedMemory;
	endMatrixA = startMatrixA + nSize*nSize - 1;
	startMatrixB = endMatrixA + 1;
	endMatrixB = startMatrixB + nSize*nSize - 1;
	startMatrixC = endMatrixB + 1;
	endMatrixC = startMatrixC + nSize*nSize - 1;

	/*printf("Address shared memory sma: %p\n", startMatrixA);
	printf("Address shared memory ema: %p\n", endMatrixA);
	printf("Address shared memory smb: %p\n", startMatrixB);
	printf("Address shared memory emb: %p\n", endMatrixB);
	printf("Address shared memory smc: %p\n", startMatrixC);
	printf("Address shared memory emc: %p\n", endMatrixC);*/

	initSeed();
	populateMatrix(startMatrixA, nSize);
	populateMatrix(startMatrixB, nSize);

	/*printMatrix(startMatrixA, nSize);
	printf("\n");
	printMatrix(startMatrixB, nSize);
	printf("\n");*/
	//printMatrix(startMatrixC, nSize);
	//printf("\n");

	//for(int i = 0; i < nSize*nSize*nSize; i++) {
	//	printf("%d ",*(startSharedMemory+i));
	//}
	//	printf("\n");

	sizeByProcess = nSize/numberOfProcesses;
	int i = 0;

	for(i = 0; i < numberOfProcesses; i++) {
		pid = fork();
		if(pid == -1) {
			printf("Error: the new process could not be created.\n");
			exit(EXIT_FAILURE);
		}
		if(pid == 0) {
			//printf("Hello from process %d\n", i+1);
			//printf("send sma: %p, smb: %p, smc: %p, nsize: %d, i: %d, size: %d\n", startMatrixA, startMatrixB, startMatrixC, nSize, i, sizeByProcess);
			multiply(startMatrixA, startMatrixB, startMatrixC, nSize, i, sizeByProcess);
			break;
		}
		else {
			//sleep(5);
			while(wait(NULL)>0);
		}
	}
	
	if(pid != 0) {

		/*printMatrix(startMatrixC, nSize);
		printf("\n");*/

		data = shmdt(startSharedMemory);

		if(data == -1) {
			printf("Error: the shared memory could not be dettached.\n");
			exit(EXIT_FAILURE);	
		}

		data = shmctl(shmid, IPC_RMID, NULL);

		if(data == -1) {
			printf("Error: the shared memory could not be deleted.\n");
			exit(EXIT_FAILURE);	
		}

	}

	return EXIT_SUCCESS;

}

void initSeed(void) {
	srand(time(NULL));
}

void populateMatrix(int* matrix, int size) {
	int i;
	int numberOfElements = size * size;
	for(i = 0; i < numberOfElements; i++) {
		*(matrix+i) = rand() % MAX_VALUE + MIN_VALUE;
	}
}

void printMatrix(int* matrix, int size) {
	int i;
	int numberOfElements = size * size;
	for(i = 0; i < numberOfElements; i++) {
		printf("%d",*(matrix+i));
		if((i+1)%size == 0) {
			printf("\n");
		}
		else {
			printf(" ");
		}
	}
}

void multiply(int* matrixA, int* matrixB, int* matrixC, int size, int numberOfProcess, int sizeByProcess) {
	int i, j, k;
	int start = numberOfProcess * sizeByProcess;
	int end = start + sizeByProcess;
	int index = start * size;
	//printf("received sma: %p, smb: %p, smc: %p, nsize: %d, i: %d, size: %d\n", matrixA, matrixB, matrixC, size, numberOfProcess, sizeByProcess);
	//printf("start: %d, end: %d, index: %d\n", start, end, index);
	/*for(i = start; i < end; i++) {
		for(k = 0; k < size; k++) {
			printf("%d",*(matrixA+i*size+k));
			if((i*size+k+1)%size == 0) {
				printf("\n");
			}
			else {
				printf(" ");
			}
		}
	}
	for(j = start; j < end; j++) {
		for(k = 0; k < size; k++) {
			printf("%d",*(matrixB+j*size+k));
			if((j+1)%size == 0) {
				printf("\n");
			}
			else {
				printf(" ");
			}
		}
	}*/
	for(i = start; i < end; i++) {
		for(j = 0; j < size; j++) {
			int value = 0;
			for(k = 0; k < size; k++) {
				int offsetMatrixA = size*i+k;
				int offsetMatrixB = size*k+j;
				int valueA = *(matrixA+offsetMatrixA);
				int valueB = *(matrixB+offsetMatrixB);
				value += valueA * valueB;
				//printf("proc: %d, offsetA: %d, offsetB: %d, valueA: %d, valueB: %d, value: %d\n",numberOfProcess, offsetMatrixA, offsetMatrixB, valueA,valueB, value);

			}
			*(matrixC+index) = value;
			index++;
		}
	}
}
