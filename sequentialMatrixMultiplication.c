#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void initRandom(void);
int* createMatrix(int, int);
void populateMatrix(int*, int);
void printMatrix(int*, int);
void multiply(int*, int*, int*, int);
void destroyMatrix(int*);

int POPULATE_RANDOM = 1;
int NO_POPULATE_RANDOM = 0;
int MIN_VALUE = 1;
int MAX_VALUE = 4;

int main(int argc, char* argv[]) {

	int* matrixA = NULL;
	int* matrixB = NULL;
	int* matrixC = NULL;
	int nSize = 0;

	if(argc!=2) {
		printf("Unexpected number of arguments.\n");
		printf("Usage: sequentialMatrixMultiplication n, where n is the size of the matrix.\n");
		exit(EXIT_FAILURE);
	}

	int data = sscanf(argv[1],"%d", &nSize);
	
	if(data!=1) {
		printf("N must be an integer.\n");
		exit(EXIT_FAILURE);
	}	 

	initRandom();

	matrixA = createMatrix(POPULATE_RANDOM, nSize);
	matrixB = createMatrix(POPULATE_RANDOM, nSize);
	matrixC = createMatrix(NO_POPULATE_RANDOM, nSize);

	/*printf("Matrix 1\n");
	printMatrix(matrixA, nSize);
	printf("Matrix 2\n");
	printMatrix(matrixB, nSize);
	printf("Matrix 3\n");
	printMatrix(matrixC, nSize); */
	multiply(matrixA, matrixB, matrixC, nSize);
	//printMatrix(matrixC, nSize);
	destroyMatrix(matrixA);
	destroyMatrix(matrixB);
	destroyMatrix(matrixC);

	return 0;

}

void initRandom(void) {
	srand(time(NULL));
}

int * createMatrix(int populate, int size) {
	int * matrix  = malloc(size * size * sizeof(int));
	if(matrix == NULL) {
		printf("An error ocurred allocating memory for a matrix: malloc failed.\n");
		exit(EXIT_FAILURE);
	}
	if(populate == POPULATE_RANDOM) {
		populateMatrix(matrix, size);
	}
	return matrix;
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

void multiply(int* matrixA, int* matrixB, int* matrixC, int size) {
	int i, j, k, index = 0;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			int value = 0;
			for(k = 0; k < size; k++) {
				int offsetMatrixA = size*i+k;
				int offsetMatrixB = size*k+j;
				int valueA = *(matrixA+offsetMatrixA);
				int valueB = *(matrixB+offsetMatrixB);
				value += valueA * valueB;
			}
			*(matrixC+index) = value;
			index++;
		}
	}
}

void destroyMatrix(int* matrix) {
	free(matrix);
}
