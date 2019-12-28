#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>

typedef struct{
    int row;
    int col;
    int (*sudoku_grid)[9];
}parameters;

void *validateSubgrid(void *);
void *validateRows(void *);
void *validateCols(void *);
void *validateRows2(void *);
void *validateCols2(void *);
int readSudokuGrid(int (*grid)[9], FILE *);
pthread_mutex_t lock;
clock_t start, end;
double totaltime;
int rq = -1;
int cq = -1;

#define NTHREADS 11
#define NTHREADS2 27

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "no command given for threading option\n");
        exit(EXIT_FAILURE);
    }
    if((strcmp(argv[1], "1") != 0) && (strcmp(argv[1], "2") != 0)){
        printf("proper threading instructions were not given\n");
        exit(0);
    }
    
    int sudoku_grid[9][9];
    
    FILE *fp = fopen("input.txt", "r"); /* provided input.txt should not have an answer I have included a seperate txt called input2.txt with a Yes solution*/
    if(fp == NULL)
    {
        printf("cannot open file\n");
        exit(0);
    }
    printf("BOARD STATE IN input.txt:\n");
    
    
    /* Read the sudoku grid to validate*/
    fseek(fp, 1, SEEK_CUR);
    
    /* Initalize parameters for subgrid evaluation threads */
    parameters *data[9];
    int row, col, i = 0;
    for(row = 0; row < 9; row += 3)
    {
        for(col = 0; col < 9; col += 3, ++i)
        {
            data[i] = (parameters *)malloc(sizeof(parameters));
            if(data[i] == NULL){
                int err = errno;
                puts("size of input file is incorrect");
                puts(strerror(err));
                exit(EXIT_FAILURE);
            }
            data[i]->row = row;
            data[i]->col = col;
            data[i]->sudoku_grid = sudoku_grid;
        }
    }
    
    /* allows max amount of threads to exist in the following process*/
    pthread_t tid[NTHREADS2];
    
    int p, j, h, retCode, check_flag = 0, t_status[NTHREADS2];
    
        start=clock();
    pthread_mutex_init(&lock, NULL);
        if(readSudokuGrid(sudoku_grid, fp)){
            puts("something happened reading the grid from the file");
            exit(EXIT_FAILURE);
        }
        
        /* Create 9 threads to check if each 3x3 subgrid is valid */
        for(p = 0; p < 9; ++p){
            if(retCode = pthread_create(&tid[p], NULL, validateSubgrid, (void *)data[p])){
                fprintf(stderr, "Error - pthread_create() return code: %d\n", retCode);
                exit(EXIT_FAILURE);
            }
        }
    if(strcmp(argv[1], "2") == 0){
        /* create 9 threads for checking through the rows to see if they are valid*/
        for(p = 9; p < 18; ++p){
            rq++;
            /*printf("%d", rq);*/
        if(retCode = pthread_create(&tid[p], NULL, validateRows2, (void *)data[0])){
            fprintf(stderr, "Error - pthread_create() return code: %d\n", retCode);
            exit(EXIT_FAILURE);
        }
            }
        for(p=18; p<27; ++p){
            cq++;
            /*printf("%d", cq);*/
        if(retCode = pthread_create(&tid[p], NULL, validateCols2, (void *)data[0])){
            fprintf(stderr, "Error - pthread_create() return code: %d\n", retCode);
            exit(EXIT_FAILURE);
        }
            }
        
        /*we clean the threads by invoking them all through join*/
        for(j = 0; j < NTHREADS2; ++j){
            if(retCode = pthread_join(tid[j], (void *)&t_status[j])){
                fprintf(stderr, "Error - pthread_join() return code: %d\n", retCode);
                exit(EXIT_FAILURE);
            }
        }
        
        /* check each threads flags to see if we have a valid sudoku board or not*/
        for(h = 0; h < NTHREADS2; ++h){
            if(t_status[h]){
                check_flag = 1;
                break;
            }
        }
    }
    if(strcmp(argv[1], "1") == 0){
    /*create only 1 thread to go through multiple times*/
        if(retCode = pthread_create(&tid[9], NULL, validateRows, (void *)data[0])){
            fprintf(stderr, "Error - pthread_create() return code: %d\n", retCode);
            exit(EXIT_FAILURE);
        }
        if(retCode = pthread_create(&tid[10], NULL, validateCols, (void *)data[0])){
            fprintf(stderr, "Error - pthread_create() return code: %d\n", retCode);
            exit(EXIT_FAILURE);
        }
        
        /*join the threads to clean*/
        for(j = 0; j < NTHREADS; ++j){
            if(retCode = pthread_join(tid[j], (void *)&t_status[j])){
                fprintf(stderr, "Error - pthread_join() return code: %d\n", retCode);
                exit(EXIT_FAILURE);
            }
        }
        
        /*check if we have a valid sudoku or not*/
        for(h = 0; h < NTHREADS; ++h){
            if(t_status[h]){
                check_flag = 1;
                break;
            }
        }
    }
        end = clock();
    pthread_mutex_destroy(&lock);
    totaltime = ((double)(end - start)) / CLOCKS_PER_SEC;
        if(check_flag){
            printf("SOLUTION: NO (%f)\n", totaltime);
        }else{
            printf("SOLUTION: YES (%f)\n", totaltime);
        }
        check_flag = 0;
    
    /*clean and free all things taking up memory on the code*/
    int k;
    for(k = 0; k < 9; ++k){
        free(data[k]);
    }
    fclose(fp);
    
    return 0;
}

/* Checks if the subgrids are valid.*/

void *validateSubgrid(void *data){
    int digit_check[10] = {0};
    parameters *params = (parameters *)data;
    int i, j;
    for(i = params->row; i < params->row + 3; ++i){
        for(j = params->col; j < params->col + 3; ++j){
            if(digit_check[params->sudoku_grid[i][j]] == 1){
                return (void *)-1; // Invalid sudoku subgrid
            }
            digit_check[params->sudoku_grid[i][j]] = 1;
        }
    }
    return (void *)0; // Valid sudoku subgrid
}

/*checks whether the rows are valid for the board or not*/

void *validateRows(void *data){
    int digit_check[10] = {0};
    parameters *params = (parameters *)data;
    int i, j;
    for(i = 0; i < 9; ++i){
        for(j = 0; j < 9; ++j){
            if(digit_check[params->sudoku_grid[i][j]] == 1){
                return (void *)-1;
            }
            digit_check[params->sudoku_grid[i][j]] = 1;
        }
        /*reinstates thread for following row*/
        memset(digit_check, 0, sizeof(int)*10);
    }
    return (void *)0;
}

void *validateRows2(void *data){
    int digit_check[10] = {0};
    parameters *params = (parameters *)data;
    int j;
        for(j = 0; j < 9; ++j){
            if(digit_check[params->sudoku_grid[rq][j]] == 1){
                return (void *)-1;
            }
            digit_check[params->sudoku_grid[rq][j]] = 1;
        }
 /*   pthread_mutex_lock(&lock);
    rq++;
    pthread_mutex_unlock(&lock); */
    return (void *)0;
}

/* check whether the columns are valid or not*/


void *validateCols(void *data){
    int digit_check[10] = {0};
    parameters *params = (parameters *)data;
    int i, j;
    for(i = 0; i < 9; ++i){
        for(j = 0; j < 9; ++j){
            if(digit_check[params->sudoku_grid[j][i]] == 1){
                return (void *)-1;
            }
            digit_check[params->sudoku_grid[j][i]] = 1;
        }
        /*reinstates the thread for the next column*/
        memset(digit_check, 0, sizeof(int)*10);
    }
    return (void *)0;
}

void *validateCols2(void *data){
    int digit_check[10] = {0};
    parameters *params = (parameters *)data;
    int j;
        for(j = 0; j < 9; ++j){
            if(digit_check[params->sudoku_grid[j][cq]] == 1){
                return (void *)-1;
            }
            digit_check[params->sudoku_grid[j][cq]] = 1;
        }
   /* pthread_mutex_lock(&lock);
    cq++;
    pthread_mutex_unlock(&lock); */
    return (void *)0;
}

/*reads the sudoku file to ensure it's proper and we get all the data from the file to be used later in the code*/
int readSudokuGrid(int (*grid)[9], FILE *fp){
    fseek(fp, 0, SEEK_SET);
    
    
    char entry;
    int i = 0, j = 0, totalValues = 0;
    while((fread(&entry, 1, 1, fp)) > 0 && totalValues < 81){
        if(entry != ' '){ /*ignores any spaces or new lines in input.txt*/
        if(entry != '\n'){
            if(isdigit(entry)){
                ++totalValues;
                printf("%c ",entry);
                grid[i][j] = entry - '0';
                ++j;
                if(j == 9){
                    printf("\n");
                    j = 0;
                    ++i;
                }
            }
            else{
                return -1; /*if error*/
            }
        }
        }
    }
    
    return 0;
}
