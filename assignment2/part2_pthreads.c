#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//define barrier
typedef struct {
    pthread_mutex_t c;
    pthread_cond_t ok;
    int count;} blue_barrier_t;

//define thread_data
struct thread_data{
    int thread_id;
    int *board;
};

//create global varibles
blue_barrier_t b;
int n, t, k, max_itrs, NUM_THREADS;
int stop = 0;

//define barrier initialize function
void blue_barrier_init(blue_barrier_t *b) {
    b->count = 0;
    pthread_mutex_init(&(b->c), NULL);
    pthread_cond_init(&(b->ok), NULL);
}
//define barrier destroy function
void blue_barrier_destroy(blue_barrier_t *b) {
    pthread_mutex_destroy (&(b->c));
    pthread_cond_destroy (&(b->ok));

}
//define barrier wait function
void blue_barrier_wait (blue_barrier_t *b) {
    pthread_mutex_lock(&b->c);
    b->count += 1;
    if(b->count == NUM_THREADS){
        b->count = 0;
        pthread_cond_broadcast(&b->ok);
        pthread_mutex_unlock(&b->c);
    }else{
        pthread_cond_wait(&b->ok,&b->c);
        pthread_mutex_unlock(&b->c);
    }
}

/*initialize the board with nxn random int from 0-2*/
void boardinit(int *grid){
    int i,j;
    for(i = 0;i < n;i++){
        for(j = 0;j<n;j++){
            grid[i*n+j]=abs(rand())%3;
        }
    }
    printf("----------initial board---------\n");
    for(i = 0;i<n;i++){
        for(j = 0;j< n;j++){
            printf("%d ",grid[i*n+j]);
        }printf("\n");
    }
    printf("--------------------------------\n");
}

/* white = 0, red = 1, blue = 2, red just move in = 3, blue just move in = 4just move out = 4*/
void *RedBlueComputation(void *threadarg){
    int i, j, a, p, q;
    int n_itrs = 0;
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    int taskid = my_data->thread_id;
    int *board = my_data->board;
    int subsize = n/NUM_THREADS;//number of rows for each threads
    float percent = (float)k/(float)100;//termination specification
    //parameters for termination check
    p = (t*t)/NUM_THREADS;
    q = (t*t)%NUM_THREADS;
    int tilesize = (n/t)*(n/t);
    int red, blue;
    if(taskid < q){
        for(a = taskid*(p+1); a < (taskid+1)*(p+1); a++){
            red = 0;
            blue = 0;
            for(i = (a/t)*(n/t); i<((a/t)+1)*(n/t); i++){
                for(j = (a%t)*(n/t); j<((a%t)+1)*(n/t); j++){
                    if(board[i*n+j] == 1){red ++;}
                    if(board[i*n+j] == 2){blue ++;}
                }
            }if(((float)red/(float)tilesize)>=percent || ((float)blue/(float)tilesize)>=percent){
                stop = 1;
                printf("tile number %d satisfied the termination requirement after %d iterations , red: %d, blue: %d.\n", a, n_itrs, red, blue);
            }

        }
    }else{
        for(a = q*(p+1)+(taskid-q)*p; a<q*(p+1)+(taskid-q)*p+p; a++){
            red = 0;
            blue = 0;
            for(i = (a/t)*(n/t); i<((a/t)+1)*(n/t); i++){
                for(j = (a%t)*(n/t); j<((a%t)+1)*(n/t); j++){
                    if(board[i*n+j] == 1){red ++;}
                    if(board[i*n+j] == 2){blue ++;}
                }
            }if(((float)red/(float)tilesize)>=percent || ((float)blue/(float)tilesize)>=percent){
                stop = 1;
                printf("tile number %d satisfied the termination requirement after %d iterations, red: %d, blue: %d.\n", a, n_itrs, red, blue);
            }
        }
    }
    blue_barrier_wait(&b);
    while(stop == 0 && n_itrs < max_itrs){
        n_itrs ++;
        //red movement
        for(i = 0;i<subsize;i++){
            for(j = 0;j<n-1;j++){
                if(board[(taskid*subsize+i)*n+j] == 1 && board[(taskid*subsize+i)*n+j+1] == 0){
                    board[(taskid*subsize+i)*n+j] = 4;
                    board[(taskid*subsize+i)*n+j+1] = 3;
                }
            }if(board[(taskid*subsize+i)*n+n-1] == 1 && board[(taskid*subsize+i)*n] == 0){
                board[(taskid*subsize+i)*n+n-1] = 4;
                board[(taskid*subsize+i)*n] = 3;
            }
        }
        //reset elements
        for(i = 0;i<subsize;i++){
            for(j = 0;j<n;j++){
                if(board[(taskid*subsize+i)*n+j] == 4){
                    board[(taskid*subsize+i)*n+j] = 0;}
                if(board[(taskid*subsize+i)*n+j] == 3){
                    board[(taskid*subsize+i)*n+j] = 1;}
            }
        }
        blue_barrier_wait(&b);
        //blue movement
        for(i = 0;i<subsize;i++){
            if(taskid*subsize+i != n-1){
                for(j = 0;j<n;j++){
                    if(board[(taskid*subsize+i)*n+j] == 2 && board[(taskid*subsize+i+1)*n+j] == 0){
                        board[(taskid*subsize+i)*n+j] = 4;
                        board[(taskid*subsize+i+1)*n+j] = 3;
                    }
                } 
            }else{
                for(j = 0;j<n;j++){
                    if(board[(n-1)*n+j] == 2 && board[j] == 0){
                        board[(n-1)*n+j] = 4;
                        board[j] = 3;
                    }
                }
            }   
        }
        blue_barrier_wait(&b);
        //reset all elements
        for(i = 0;i<subsize;i++){
            for(j = 0;j<n;j++){
                if(board[(taskid*subsize+i)*n+j] == 4){
                    board[(taskid*subsize+i)*n+j] = 0;}
                if(board[(taskid*subsize+i)*n+j] == 3){
                    board[(taskid*subsize+i)*n+j] = 2;}
            }
        }
        blue_barrier_wait(&b);
        //seperate tiles to each threads and calculate percentage
        if(taskid < q){
            for(a = taskid*(p+1); a < (taskid+1)*(p+1); a++){
                red = 0;
                blue = 0;
                for(i = (a/t)*(n/t); i<((a/t)+1)*(n/t); i++){
                    for(j = (a%t)*(n/t); j<((a%t)+1)*(n/t); j++){
                        if(board[i*n+j] == 1){red ++;}
                        if(board[i*n+j] == 2){blue ++;}
                    }
                }if(((float)red/(float)tilesize)>=percent || ((float)blue/(float)tilesize)>=percent){
                    stop = 1;
                    printf("tile number %d satisfied the termination requirement after %d iterations , red: %d, blue: %d.\n", a, n_itrs, red, blue);
                }

            }
        }else{
            for(a = q*(p+1)+(taskid-q)*p; a<q*(p+1)+(taskid-q)*p+p; a++){
                red = 0;
                blue = 0;
                for(i = (a/t)*(n/t); i<((a/t)+1)*(n/t); i++){
                    for(j = (a%t)*(n/t); j<((a%t)+1)*(n/t); j++){
                        if(board[i*n+j] == 1){red ++;}
                        if(board[i*n+j] == 2){blue ++;}
                    }
                }if(((float)red/(float)tilesize)>=percent || ((float)blue/(float)tilesize)>=percent){
                    stop = 1;
                    printf("tile number %d satisfied the termination requirement after %d iterations, red: %d, blue: %d.\n", a, n_itrs, red, blue);
                }
            }
        }blue_barrier_wait(&b);
    }if(taskid == 0&& n_itrs == max_itrs){
        printf("parallelize computation terminate after %d iterations\n", n_itrs);
    }    
    pthread_exit(NULL);
}

void sequential(int *seqboard){
    printf("------Start sequential computation------\n");
    int i, j, a;
    int n_itrs = 0;
    int terminate = 0;
    float percent = (float)k/(float)100;
    while (terminate == 0 && n_itrs< max_itrs){
        //check termination
        int red, blue;
        int tilesize = (n/t)*(n/t);
        for(a = 0; a< t*t; a++){
            red = 0;
            blue = 0;
            for(i = (a/t)*(n/t); i<((a/t)+1)*(n/t); i++){
                for(j = (a%t)*(n/t); j<((a%t)+1)*(n/t); j++){
                    if(seqboard[i*n+j] == 1){red ++;}
                    if(seqboard[i*n+j] == 2){blue ++;}
                }
            }if(((float)red/(float)tilesize)>=percent || ((float)blue/(float)tilesize)>=percent){
                terminate = 1;
                printf("tile number %d satisfied the termination requirement after %d iterations red: %d, blue: %d \n", a, n_itrs, red, blue);
            }
        }
        n_itrs ++;
        //red movement
        for(i = 0;i<n; i++){
            for(j = 0;j<n-1;j++){
                if(seqboard[i*n+j] == 1 && seqboard[i*n+j+1] == 0){
                    seqboard[i*n+j] = 4;
                    seqboard[i*n+j+1] = 3;
                }
            }if(seqboard[i*n+n-1] == 1 && seqboard[i*n] == 0){
                seqboard[i*n+n-1] = 4;
                seqboard[i*n] = 3;
            }
        }for(i = 0; i<n*n; i++){
            if(seqboard[i] == 4){seqboard[i] = 0;}
            if(seqboard[i] == 3){seqboard[i] = 1;}
        }
        //blue movement
        for(i = 0; i<n; i++){
            if(i < n-1){
                for(j = 0;j<n;j++){
                    if(seqboard[i*n+j] == 2 && seqboard[(i+1)*n+j] == 0){
                        seqboard[i*n+j] = 4;
                        seqboard[(i+1)*n+j] = 3;
                    }
                }
            }else{
                for(j = 0; j<n;j++){
                    if(seqboard[i*n+j] == 2 && seqboard[j] == 0){
                        seqboard[i*n+j] = 4;
                        seqboard[j] = 3;
                    }
                }
            }   
        }
        for(i = 0; i<n*n; i++){
            if(seqboard[i] == 4){seqboard[i] = 0;}
            if(seqboard[i] == 3){seqboard[i] = 2;}
        }
    }
    if(n_itrs == max_itrs){
        printf("sequential computation terminate after %d iterations \n", n_itrs);
    }
    printf("Final board:\n");
    for(i = 0;i<n;i++){
        for(j = 0;j< n;j++){
            printf("%d ",seqboard[i*n+j]);
        }printf("\n");
    }
    printf("----------------------------------------\n");
}

int main(int argc, char *argv[]){
    //initialize parameters and arguments
    int rc, i, j;
    NUM_THREADS = atoi(argv[1]);
    n = atoi(argv[2]);
    t = atoi(argv[3]);
    k = atoi(argv[4]);
    max_itrs = atoi(argv[5]);
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    //initialize board and barrier
    int *board = calloc(n*n,sizeof(int));
    int *seqboard = calloc(n*n,sizeof(int));
    boardinit(board);
    //copy the board for sequential evaluation
    for(i = 0;i<n*n; i++){
        seqboard[i] = board[i];
    }
    //initialize barrier
    blue_barrier_init(&b);
    //create threads and passing messages
    printf("-----------start parallelize----------\n");
    for(i=0;i<NUM_THREADS;i++) {
        thread_data_array[i].thread_id = i;
        thread_data_array[i].board = board;
        printf("Creating thread %d......\n", i);
        rc = pthread_create(&threads[i], NULL, RedBlueComputation, (void *)
                            &thread_data_array[i]);
        
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    //join all the threads
    for(i = 0;i<NUM_THREADS;i++){
        pthread_join(threads[i], NULL);
    }
    //print the board
    printf("Final board:\n");
    for(i = 0;i<n;i++){
        for(j = 0;j< n;j++){
            printf("%d ",board[i*n+j]);
        }printf("\n");
    }printf("----------------------------------------\n");
    //free all the memory and destroy the barriers
    sequential(seqboard);
    free(board);
    free(seqboard);
    blue_barrier_destroy(&b);
    pthread_exit(NULL);
    
    
}
