//Program for simulating the lottery

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 5
#define JACKPOT 500000000
//#define RUNS 1000

typedef struct Balls {
    int whiteballs[SIZE];
    int redball;
} Balls;

typedef struct Flags {
    int vflag = 0; 
    int fflag = 0; 
    int dflag = 0;
} Flags;

int runs = 1000;

/*int compare_ball(int myWhiteball, int drawnWhiteball){
    if(myWhiteball == drawnWhiteball)
        return 1;
    else
        return 0;
}*/

int calculate_payout(int whiteMatches, int redMatch){
    int payout = 0;
    switch(whiteMatches){
        case 0:
            break;
        case 1:
            if(redMatch == 1)
                payout = 4;
            break;
        case 2:
            if(redMatch == 1)
                payout = 7;
            break;
        case 3:
            if(redMatch == 1)
                payout = 100;
            else
                payout = 7;
            break;
        case 4:
            if(redMatch == 1)
                payout = 10000;
            else
                payout = 100;
            break;
        case 5:
            if(redMatch == 1)
                payout = JACKPOT;
            else
                payout = 1000000;
            break;
    }
    return payout;
}

int get_payout(Balls myBalls, Balls drawnBalls){
    int whiteMatches = 0, redMatch = 0;
    for(int i = 0; i < SIZE; i++){
        if(myBalls.whiteballs[i] == drawnBalls.whiteballs[i])
            whiteMatches++;
    }
    if(myBalls.redball == drawnBalls.redball)
        redMatch = 1;
    return calculate_payout(whiteMatches, redMatch);
}

void generate_balls(Balls* myBalls){
    for(int i = 0; i < SIZE; i++){
        myBalls->whiteballs[i] = (rand() % 69) + 1;
    }
    myBalls->redball = (rand() % 26) + 1;
}

void print_balls(Balls myBalls){
    for(int i = 0; i < SIZE; i++){
        printf("%d ", myBalls.whiteballs[i]);
    }
    printf("%d\n", myBalls.redball);
}

int add_payouts(int payouts[]){
    int sum = 0;
    for(int i = 0; i < runs; i++){
        sum += payouts[i];
    }
    return sum;
}

void get_flags(Flags* flags, int argc, char* argv[]){
    int opt;
    while((opt = getopt(argc, argv, "vfd")) != -1){
        switch(opt){
            case 'v':
                flags->vflag = 1;
                break;
            case 'f':
                flags->fflag = 1;
                break;
            case 'd':
                flags->dflag = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-vfd] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[]){

    Flags flags;
    get_flags(&flags, argc, argv);
    if(flags.dflag == 1)
        printf("flags: %d %d %d\n", flags.vflag, flags.fflag, flags.dflag);
    if(flags.vflag == 1 || flags.fflag == 1 || flags.dflag == 1)
        runs = atoi(argv[2]);
    else
        runs = atoi(argv[1]);

    Balls myBalls;
    Balls drawnBalls;
    int payouts[runs];

    time_t t;
    clock_t start, end;
    double cpu_time_used;

    srand((unsigned) time(&t));
    
    start = clock();
    for(int i = 0; i < runs; i++){
        generate_balls(&myBalls);
        generate_balls(&drawnBalls);
        payouts[i] = get_payout(myBalls, drawnBalls);
        if(flags.vflag == 1){
            printf("My balls: \t");
            print_balls(myBalls);
            printf("Drawn balls: \t");
            print_balls(drawnBalls);
            printf("Payout: $%d\n", payouts[i]);
            printf("-------------------------------\n");
        }
    }
    int total = add_payouts(payouts);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Total payout: $%d\n", total);
    
    if(flags.fflag == 1){
        int file = open(argv[3], O_WRONLY | O_APPEND | O_CREAT, 0744);
        char totalString[10];
        sprintf(totalString, "%d\n", total);
        int written = write(file, totalString, strlen(totalString));
        if(flags.dflag == 1){
            printf("Bytes written: %d\n", written);
        }
        close(file);
    }
    if(flags.dflag == 1)
        printf("CPU time: %f\n", cpu_time_used);

    return 0;
}