//
//  main.c
//  test
//
//  Created by yuanqi on 2018/4/7.
//  Copyright © 2018年 yuanqi. All rights reserved.
//
/* white = 0, red = 1, blue = 2,
 red just moved in = 3 and blue just moved in = 4
 red or blue (in the first row or column) just moved out = 5*/

#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int* boardinit(int *grid, int n){
    int i,j;
    grid = (int*)malloc(sizeof(int)*n*n);
    for(i = 0;i < n;i++){
        for(j = 0;j<n;j++){
            grid[i*n+j]=abs(rand())%3;
        }
    }
    printf("The initial board: \n");
    for(i = 0;i<n;i++){
        printf("-");
    }
    printf("\n");
    for(i = 0;i<n;i++){
        for(j = 0;j< n;j++){
            printf("%d",grid[i*n+j]);
        }printf("\n");
    }
    for(i = 0;i<n;i++){
        printf("-");
    }
    printf("\n");
    return grid;
    /*initialize the board with nxn random int from 0-2*/
}
int sequential(int * board, int n,int t,int k,int MAX_ITRS) {
    /*Self check, the result can be printed and compared with sequential part*/
    printf("---------------sequential computing begins...--------------\n");
    double c = (double)k/(double)100;
    int i,j,x;
    int p,q;
    bool finished = false;
    int n_itrs = 0;
    int redcount, bluecount;
    int grid[n][n];     /* grid[row][col] */
    for(i = 0;i<n;i++){
        for(j = 0;j<n;j++){
            grid[i][j] = board[i*n+j];
        }
    }
    while (!finished && n_itrs < MAX_ITRS){
        /* count the number of red and blue in each tile and check if the computation can be terminated
         sequential computing is computing with 2-D array*/
        n_itrs++;
        printf("---Sequential: iteration NO. %d---\n",n_itrs);
        /* red color movement */
        for (i=0;i<n;i++){
            for(j=0;j<n-1;j++){
                if(grid[i][j]==1&&grid[i][j+1]==0){
                    grid[i][j] = 5;
                    grid[i][j+1] = 3;
                }
            }
            if(grid[i][n-1]==1&&grid[i][0]==0){
                grid[i][n-1] = 5;
                grid[i][0] = 3;
            }
        }
        /*reset all element*/
        for(i = 0;i<n;i++){
            for(j = 0;j<n;j++){
                if(grid[i][j]==3){grid[i][j] = 1;}
                if(grid[i][j]==5){grid[i][j] = 0;}
            }
        }
        /* blue color movement */
        for (j=0;j<n;j++){
            for(i=0;i<n-1;i++){
                if(grid[i][j]==2&&grid[i+1][j]==0){
                    grid[i][j] = 5;
                    grid[i+1][j] = 4;
                }
            }
            if(grid[n-1][j]==2&&grid[0][j]==0){
                grid[n-1][j] = 5;
                grid[0][j] = 4;
            }
        }
        /*reset all element*/
        for(i = 0;i<n;i++){
            for(j = 0;j<n;j++){
                if(grid[i][j]==4){grid[i][j] = 2;}
                if(grid[i][j]==5){grid[i][j] = 0;}
                printf("%d",grid[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        /*check if color percentage meet the requirement*/
        for(i = 0;i<t;i++){
            for(j = 0;j<t;j++){
                redcount = 0;
                bluecount = 0;
                for(p = i*(n/t);p<(i+1)*(n/t);p++){
                    for(q = j*(n/t);q<(j+1)*(n/t);q++){
                        if(grid[p][q]==1){redcount++;}
                        if(grid[p][q]==2){bluecount++;}
                    }
                }
                if(redcount>c*(n/t)*(n/t)||bluecount>c*(n/t)*(n/t)){
                    finished = true;
                    /*print out the tiles that meet the requirement*/
                    for(p = i*(n/t);p<(i+1)*(n/t);p++){
                        for(q = j*(n/t);q<(j+1)*(n/t);q++){
                            printf("%d",grid[p][q]);
                        }
                        printf("\n");
                    }
                    for(x = 0;x<n/t;x++){
                        printf("-");
                    }
                    printf("\n");
                }
            }
        }
    }
    if(finished == false){
        printf(" Reach max iteration times......... \n sequential computing terminates...\n-----------------------------------------------------------\n");
    }else{
        printf("after %d Sequential iteration...\n", n_itrs);
        printf("tiles that meet the requirement are listed above \n-----------------------------------------------------------\n");
    }
    return 0;
}
int main (int argc, char** argv) {
    int n,t,k,MAX_ITRS;
    int i, j, p, q, tg, m;
    int grid[n][n];
    int* board= (int*)malloc(sizeof(int)*n*n),*board_copy;
    int* subboard;
    int stop = 0, n_itrs=0;
    if(argc != 5){
        printf("Argument count: %d doesn't meet the requirement\n",argc);
        printf("enter >>>mpirun -np [procs number] <program_name> [Grid size N] [Tile size t] [Required percentage c] [Max iteration times]\n");
        printf("sample: mpirun -np 2 main 6 2 60 10\n");
        MPI_Finalize();
        exit(0);
    }
    /*get input parameter and initialize all parameters that will be used in the project*/
    n = atoi(argv[1]);
    t = atoi(argv[2]);
    k = atoi(argv[3]);
    tg = (n/t);
    MAX_ITRS = atoi(argv[4]);
    double c = (double)k/(double)100;
    int* yitiao=(int*)malloc(sizeof(int)*n*2);
    // initialize MPI environment and get the total number of processes and process id.
    int myid,numprocs;
    MPI_Status status;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    /*if processor number grearer than tile number, each processor get 1 tile and leave the rest blank*/
    if(numprocs>t){numprocs = t;}
    if(myid == 0){
        int red, blue;
        int ct1, ct2;
        /* Initialize board*/
        board = boardinit(board, n);
        board_copy=(int*)malloc(sizeof(int)*n*n);
        for(i=0;i<n;i++){
            for (j=0; j<n; j++) {
                board_copy[i*n+j]=board[i*n+j];
            }
        }
        /*judge if the processor is more than 1 to decide which function will be used*/
        if(numprocs ==1){
            printf("Only 1 process \n");
            sequential(board,n,t,k,MAX_ITRS);
        }
        else{
            printf("start self checking......\n");
            sequential(board_copy,n,t,k,MAX_ITRS);
            
            printf("More than 1 process, computing in parallel...\n");
            q = t%numprocs;
            /* patition tasks to every procs*/
            for(i = 0;i<numprocs;i++){
                p = t/numprocs;
                /*patition all tasks in one time, if p<1 means processor number greater than tile number, so not all processors are used, if p>=1 means at least 1 tile is send to each tile, and for the first q processors,m(p+1)tiles are assigned to them, rest of the processors only need n tiles, in this way, the assigned tiles to each processor are continous, which can reduce the communication between processors*/
                if(i<q){
                    p = p+1;
                    MPI_Send(&board[i*p*tg*n], p*tg*n, MPI_INT, i, 1, MPI_COMM_WORLD);/*send tiles to processor*/
                    MPI_Send(&p,1,MPI_INT,i,2,MPI_COMM_WORLD); /*send number of tiles to the processor*/
                }else{
                    MPI_Send(&board[(q+i*p)*tg*n], p*tg*n, MPI_INT, i, 1, MPI_COMM_WORLD);
                    MPI_Send(&p,1,MPI_INT,i,2,MPI_COMM_WORLD);
                    
                    /*q processors has been assigned 1 more tile, start from (q+i*p) tile*/
                }
            }
            for(i = 1;i<numprocs;i++){
                MPI_Send(&tg,1,MPI_INT,i,3,MPI_COMM_WORLD);
            }
            MPI_Recv(&p,1,MPI_INT,0,2,MPI_COMM_WORLD,&status);
            subboard = (int*)malloc(sizeof(int)*(p*tg)*n);
            MPI_Recv(&subboard[0],p*tg*n,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            while(n_itrs < MAX_ITRS && stop == 0){
                n_itrs++;
                /*red movement dont need communication between processors*/
                for(i = 0;i<p*tg;i++){
                    for (j = 0;j<n-1;j++){
                        if(subboard[i*n+j]==1 &&subboard[i*n+j+1]==0){
                            subboard[i*n+j]=5;
                            subboard[i*n+j+1]=3;
                        }
                    }
                    if(subboard[i*n+n-1]==1&&subboard[i*n]==0){
                        subboard[i*n+n-1]=5;
                        subboard[i*n]=3;
                    }
                }
                for(i = 0;i<p*tg;i++){
                    for(j = 0;j<n;j++){
                        if(subboard[i*n+j]==5){subboard[i*n+j]=0;}
                        if(subboard[i*n+j]==3){subboard[i*n+j]=1;}
                    }
                }
                /*blue movement*/
                /*grids that can not be affected by other processor*/
                for(i = 0;i<p*tg-1;i++){
                    for(j = 0;j<n;j++){
                        if(subboard[i*n+j]==2&&subboard[(i+1)*n+j]==0){
                            subboard[i*n+j] = 5;
                            subboard[(i+1)*n+j] = 4;
                        }
                    }
                }
                /*send data to upper processor and receive data from lower processor*/
                MPI_Send(&subboard[0], n, MPI_INT, numprocs-1, 4, MPI_COMM_WORLD);
                MPI_Recv(&yitiao[0],n,MPI_INT, myid+1,4,MPI_COMM_WORLD,&status);
                /*lines that need data from other processor*/
                for(i = 0;i<n;i++){
                    if(subboard[(p*tg-1)*n+i]==2&&yitiao[i]==0){
                        subboard[(p*tg-1)*n+i]=5;
                        yitiao[i]=4;
                    }
                }
                MPI_Send(&yitiao[0], n, MPI_INT, myid+1, 5, MPI_COMM_WORLD);
                MPI_Recv(&yitiao[n],n,MPI_INT, numprocs-1,5,MPI_COMM_WORLD,&status);
                /*get data from previous line and update the first line*/
                for(j = 0;j<n;j++){
                    if(yitiao[n+j]==4){
                        subboard[j] = 2;
                    }
                }
                /*reset all the element to 0,1,2*/
                for(i = 0;i<p*tg*n;i++){
                    if(subboard[i]==5){subboard[i]=0;}
                    if(subboard[i]==4){subboard[i]=2;}
                }
                /*finish one iteration collect the subboard to print out the result*/
                MPI_Send(&subboard[0],tg*p*n,MPI_INT,0,7,MPI_COMM_WORLD);
                i = 0;
                while(i<numprocs){
                    int x =t/numprocs;
                    q =t%numprocs;
                    if(i<x)
                        MPI_Recv(&board[x*tg*n*i],(x+1)*tg*n,MPI_INT,i,7,MPI_COMM_WORLD,&status);
                    else
                        MPI_Recv(&board[(q*(x+1)+(i-q)*x)*tg*n],x*tg*n,MPI_INT,i,7,MPI_COMM_WORLD,&status);
                    i++;
                }/*print out the result after each iteration*/
                printf("---Parallel: iteration NO. %d---\n", n_itrs);
                for(i = 0;i<n;i++){
                    for(j = 0;j<n;j++){
                        printf("%d",board[i*n+j]);
                    }printf("\n");
                }printf("\n");
                /*set barrier to let all processors finish movement before checking*/
                MPI_Barrier( MPI_COMM_WORLD );
                /*calculate the percentage to see if the computing shold terminate*/
                for(i = 0;i<p;i++){
                    for(j = 0;j<t;j++){
                        red = blue = 0;
                        for (ct1 = tg*i; ct1<tg*(i + 1);ct1++){
                            for (ct2 = tg*j; ct2<tg*(j + 1); ct2++){
                                if (subboard[ct1*n+ct2] == 1)
                                    red++;
                                if (subboard[ct1*n+ct2] == 2)
                                    blue++;
                            }
                        }
                        if (red > c*tg*tg ||blue > c*tg*tg){
                            stop = 1;
                            for (ct1 = tg*i; ct1<tg*(i + 1);ct1++){
                                for (ct2 = tg*j; ct2<tg*(j + 1); ct2++){
                                    printf("%d",subboard[ct1*n+ct2]);
                                }printf("\n");
                            }printf("------\n");
                        }
                    }
                }
                MPI_Allreduce(&stop,&stop,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
            }
        }
    }else{
        /*only the processors that has been assigned job need to work*/
        if(myid<numprocs){
            int p,tg;
            MPI_Recv(&p,1,MPI_INT,0,2,MPI_COMM_WORLD,&status);
            MPI_Recv(&tg,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
            subboard = (int*)malloc(sizeof(int)*(p*tg)*n);
            int ct1, ct2;
            int red = 0;
            int blue = 0;
            MPI_Recv(&subboard[0], p*tg*n, MPI_INT, 0, 1, MPI_COMM_WORLD,&status);
            while(n_itrs < MAX_ITRS && stop == 0){
                n_itrs++;
                /*red movement*/
                for(i = 0;i<p*tg;i++){
                    for (j = 0;j<n-1;j++){
                        if(subboard[i*n+j]==1 &&subboard[i*n+j+1]==0){
                            subboard[i*n+j]=5;
                            subboard[i*n+j+1]=3;
                        }
                    }
                    if(subboard[i*n+n-1]==1&&subboard[i*n]==0){
                        subboard[i*n+n-1]=5;
                        subboard[i*n]=3;
                    }
                }/*reset elements*/
                for(i = 0;i<p*tg;i++){
                    for(j = 0;j<n;j++){
                        if(subboard[i*n+j]==5){subboard[i*n+j]=0;}
                        if(subboard[i*n+j]==3){subboard[i*n+j]=1;}
                    }
                }
                /*blue: grids that can not be affected by other processor*/
                for(i = 0;i<p*tg-1;i++){
                    for(j = 0;j<n;j++){
                        if(subboard[i*n+j]==2&&subboard[(i+1)*n+j]==0){
                            subboard[i*n+j] = 5;
                            subboard[(i+1)*n+j] = 4;
                        }
                    }
                }
                MPI_Send(&subboard[0], n, MPI_INT, myid-1, 4, MPI_COMM_WORLD);
                if(myid ==numprocs-1){
                    MPI_Recv(&yitiao[0],n,MPI_INT, 0, 4,MPI_COMM_WORLD,&status);
                }
                else{
                    MPI_Recv(&yitiao[0],n,MPI_INT, myid+1,4,MPI_COMM_WORLD,&status);
                }
                /*lines that need data from other processor*/
                for(i = 0;i<n;i++){
                    if(subboard[(p*tg-1)*n+i]==2&&yitiao[i]==0){
                        subboard[(p*tg-1)*n+i]=5;
                        yitiao[i]=4;
                    }
                }
                /*send data to different processor*/
                if(myid ==numprocs-1){
                    MPI_Send(&yitiao[0], n, MPI_INT, 0, 5, MPI_COMM_WORLD);
                }else{
                    MPI_Send(&yitiao[0], n, MPI_INT, myid+1, 5, MPI_COMM_WORLD);
                }
                MPI_Recv(&yitiao[n],n,MPI_INT, myid-1,5,MPI_COMM_WORLD,&status);
                /*get data from previous line and update the first line*/
                for(j = 0;j<n;j++){
                    if(yitiao[n+j]==4){
                        subboard[j] = 2;
                    }
                }
                /*reset all the element to 0,1,2*/
                for(i = 0;i<p*tg*n;i++){
                    if(subboard[i]==5){subboard[i]=0;}
                    if(subboard[i]==4){subboard[i]=2;}
                }
                MPI_Send(&subboard[0],tg*p*n,MPI_INT,0,7,MPI_COMM_WORLD);//sent subboard back
                MPI_Barrier( MPI_COMM_WORLD );
//                finish one iteration and start calculating percentage
                for(i = 0;i<p;i++){
                    for(j = 0;j<t;j++){
                        red = blue = 0;
                        for (ct1 = tg*i; ct1<tg*(i + 1);ct1++){
                            for (ct2 = tg*j; ct2<tg*(j + 1); ct2++){
                                if (subboard[ct1*n+ct2] == 1)
                                    red++;
                                if (subboard[ct1*n+ct2] == 2)
                                    blue++;
                            }
                        }
                        if (red > c*tg*tg ||blue > c*tg*tg){
                            for (ct1 = tg*i; ct1<tg*(i + 1);ct1++){
                                for (ct2 = tg*j; ct2<tg*(j + 1); ct2++){/*print out the tiles that meet the requirement*/
                                    printf("%d",subboard[ct1*n+ct2]);
                                }printf("\n");
                            }printf("------\n");
                            stop = 1;
                        }
                    }
                }
                MPI_Allreduce(&stop,&stop,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
            }
        }/*other processors doesn't work but need the allreduce signal to stop*/
        else{
            int stop = 0;
            while(n_itrs < MAX_ITRS && stop == 0){
                n_itrs++;
                MPI_Barrier( MPI_COMM_WORLD );
                MPI_Allreduce(&stop,&stop,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
            }
        }
    }/*processor 0 finalize the project*/
    if(myid ==0){
        if(stop ==0){
            printf("after %d Parallel iteration...\n", n_itrs);
            printf("Parallel computing terminates by reaching the iteration limit \n-----------------------------------------------------------\n");
            
        }else{
            printf("after %d Parallel iteration, board meet the percentage requirement...\n", n_itrs);
            printf("tiles that meet the requirement are listed. \n-----------------------------------------------------------\n");
        }
    }
    free(board);
    MPI_Finalize();
    return 0;
}





