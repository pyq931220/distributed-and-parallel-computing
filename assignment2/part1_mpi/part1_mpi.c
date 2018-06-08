//  Copyright © 2018年 yuanqi. All rights reserved.
#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

float* creatematrix(float* matrix, int n, int m){
    int i,j;
    printf("--------------Initial Matrix:--------------\n");
    for(i = 0;i < n;i++){
        for(j = 0;j < m;j++){
            matrix[i*m+j] = ((float)rand()/(float)(RAND_MAX)) * 1.0;
            printf("%f  ",matrix[i*m+j]);
        }printf("\n");
    }printf("-------------------------------------------\n");
    return matrix;
}
float* sequential(float* matrix, int n, int m, float* result){
    int i,j,k;
    int x = 0;
    float val;
    for(i = 0;i<n;i++){
        for(j = i+1;j<n;j++){
            val = 0.0;
            for(k = 0;k<m;k++){
                val+=matrix[i*m+k]*matrix[j*m+k];}
            result[i*(n-1)+j-1] = val;
            x++;}}return result;
}
int numresults(int n){
    int i,result = 0;
    for(i = n-1;i>0;i--){
        result += i;
    }return result;
}
int main (int argc, char** argv) {
    // initialize MPI environment and get the total number of processes and process id.
    int myid,numprocs;
    MPI_Status status;
    MPI_Request req;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    //initialize parameters and args
    int n,m,i,j,k,l;
    float val;
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    int p = n/numprocs;
    int n_itrs = numprocs/2;
    //start parallelize
    if(numprocs == 1){
        float* matrix = (float*)calloc(n*m,sizeof(float));
        matrix = creatematrix(matrix, n, m);
        printf("\n Only 1 processor, computing in sequential mode.\n");
        float* result = (float*)calloc((n-1)*(n-1),sizeof(float));
        result = sequential(matrix, n, m, result);
        printf("------------Sequential result:-------------\n");
        for(i = 0;i<n-1;i++){
            j = 0;
            while(result[i*(n-1)+j]==0){j++;}
            for(j = j;j<n-1;j++){printf("%f ",result[i*(n-1)+j]);}
            printf("\n");
        }printf("-------------------------------------------\n");
        free(result);
        free(matrix);
    }else{
        if(myid == 0){
            //initialize board for storing results
            float* par_result = (float*)calloc((n-1)*(n-1),sizeof(float));
            float* seq_result = (float*)calloc((n-1)*(n-1),sizeof(float));
            //initialize matrix with random float number
            float* matrix = (float*)calloc(n*m,sizeof(float));
            matrix = creatematrix(matrix, n, m);
            //allocate vectors to every processor and store in a sub_matrix
            float* sub_matrix = (float*)malloc(p*m*sizeof(float));
            for(i = 0;i<p*m;i++){
                sub_matrix[i] = matrix[i];
            }
            for(i = 1;i<numprocs;i++){
                MPI_Isend(&matrix[i*p*m], p*m, MPI_FLOAT, i, 1, MPI_COMM_WORLD, &req);
                MPI_Wait(&req, &status);
            }
            //calculate the inner product within 1 processor
            for(i = 0;i<p;i++){
                for(j = i+1;j<p;j++){
                    val = 0.0;
                    for(k = 0;k<m;k++){
                        val+= sub_matrix[i*m+k]*sub_matrix[j*m+k];
                    }par_result[i*(n-1)+j-1] = val;}}
            //pass submatrix to the back processors and calculate the inner product of vectors from 2 processors
            float* others_matrix = (float*)calloc(p*m,sizeof(float));
            for(i = 0;i<n_itrs;i++){
                MPI_Isend(&sub_matrix[0], p*m, MPI_FLOAT, i+1, 3, MPI_COMM_WORLD, &req);
                MPI_Wait(&req, &status);
                MPI_Irecv(&others_matrix[0], p*m, MPI_FLOAT, numprocs-1-i, 3, MPI_COMM_WORLD, &req);
                MPI_Wait(&req, &status);
                for(j = 0;j<p;j++){
                    for(k = 0;k<p;k++){
                        val = 0.0;
                        for(l = 0;l<m;l++){
                            val += sub_matrix[j*m+l]*others_matrix[k*m+l];
                        }par_result[j*(n-1)+(numprocs-i-1)*p+k-1] = val;}}}
            free(others_matrix);
            MPI_Barrier(MPI_COMM_WORLD);
            //get result from other processors
            float *returnresult = (float*)calloc((numresults(p)+n_itrs*p*p)*2, sizeof(float));
            for(k = 1;k<numprocs;k++){
                MPI_Irecv(&returnresult[0], (numresults(p)+n_itrs*p*p)*2, MPI_FLOAT, k, 2, MPI_COMM_WORLD, &req);
                MPI_Wait(&req, &status);
                for(i = 0;i<(numresults(p)+n_itrs*p*p);i++){
                    par_result[(int)returnresult[i*2]] = returnresult[i*2+1];}}
            free(returnresult);
            //print out result of parallelize computing
            printf("------------Parallel result:-------------\n");
            for(i = 0;i<n-1;i++){
                j = 0;
                while(par_result[i*(n-1)+j]==0){j++;}
                for(j = j;j<n-1;j++){printf("%f ",par_result[i*(n-1)+j]);}
                printf("\n");
            }printf("-------------------------------------------\n");
            //start sequential computing for self checking
            printf("Start Sequential computing for self checking......\n");
            seq_result = sequential(matrix, n, m, seq_result);
            printf("------------Sequential result:-------------\n");
            for(i = 0;i<n-1;i++){
                j = 0;
                while(seq_result[i*(n-1)+j]==0){j++;}
                for(j = j;j<n-1;j++){printf("%f ",seq_result[i*(n-1)+j]);}
                printf("\n");
            }printf("-------------------------------------------\n");
            //compare the result and print out number of errors if there is dismatch
            int errors = 0;
            for(i = 0;i<(n-1)*(n-1);i++){
                if(seq_result[i]!=par_result[i]){
                    errors++;}
            }printf("Finish self checking, total error: %d.\n",errors);
            free(par_result);
            free(seq_result);
            free(sub_matrix);
            free(matrix);
        }else{
            int count = 0;
            float *returnresult = (float*)malloc(sizeof(float)*(numresults(p)+n_itrs*p*p)*2);
            float* sub_matrix = (float*)malloc(p*m*sizeof(float));
            MPI_Irecv(&sub_matrix[0], p*m, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &status);
            for(i = 0;i<p;i++){
                for(j = i+1;j<p;j++){
                    val = 0.0;
                    for(k = 0;k<m;k++){
                        val+= sub_matrix[i*m+k]*sub_matrix[j*m+k];}
                    returnresult[count] = (float)((myid*p+i)*(n-1)+(myid*p+j-1));
                    returnresult[count+1] = val;
                    count += 2;}}
            float* others_matrix = (float*)calloc(p*m,sizeof(float));
            for(i = 0;i<n_itrs;i++){
                MPI_Isend(&sub_matrix[0], p*m, MPI_FLOAT, (myid+i+1)%numprocs, 3, MPI_COMM_WORLD, &req);
                MPI_Wait(&req, &status);
                MPI_Irecv(&others_matrix[0], p*m, MPI_FLOAT, (myid-1-i+numprocs)%numprocs, 3, MPI_COMM_WORLD, &req);
                MPI_Wait(&req, &status);
                for(j = 0;j<p;j++){
                    for(k = 0;k<p;k++){
                        val = 0.0;
                        for(l = 0;l<m;l++){
                            val += sub_matrix[j*m+l]*others_matrix[k*m+l];}
                        if(myid<(myid-1-i+numprocs)%numprocs){
                            returnresult[count] = (float)((j+myid*p)*(n-1)+((myid-i-1+numprocs)%numprocs)*p+k-1);
                            returnresult[count+1] = val;
                            count += 2;
                        }else{
                            returnresult[count] = (float)((((myid-i-1+numprocs)%numprocs)*p+k)*(n-1)+(j+myid*p)-1);
                            returnresult[count+1] = val;
                            count += 2;}}}
            }free(others_matrix);
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Isend(&returnresult[0], (numresults(p)+n_itrs*p*p)*2, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &status);
            free(returnresult);
            free(sub_matrix);
        }
    }
    MPI_Finalize();
    return 0;
}
