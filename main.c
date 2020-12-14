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
#define COST 2
#define RUNS 1000

typedef struct Balls {
    int whiteballs[SIZE];
    int redball;
} Balls;

enum Flags {
    VERBOSE = 1,
    TEXTFILE = 2,
    DEV = 4,
    WINNINGS = 8
};

//global variables
int runs = RUNS; //number of simulation rungs
int cost = COST; //cost of a ticket
char* file = (char*) malloc(100 * sizeof(char)); //text file for IO
int flags = 0; //program flags
int showWinningsAbove = 0; //for -w flag to show winnings above $ amount

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

int already_matched(int matched[], int ballPos){
    for(int i = 0; i < SIZE; i++){
        if(matched[i] == ballPos)
            return 1;
    }
    return 0;
}

int get_payout(Balls myBalls, Balls drawnBalls){
    int whiteMatches = 0, redMatch = 0;
    int matched[5] = {-1, -1, -1, -1, -1};
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            if(myBalls.whiteballs[i] == drawnBalls.whiteballs[j] 
                && already_matched(matched, j) != 1){
                matched[whiteMatches] = j;
                whiteMatches++;
                break;
            }
        }
    }
    if(myBalls.redball == drawnBalls.redball)
        redMatch = 1;
    return calculate_payout(whiteMatches, redMatch);
}

void generate_balls(Balls* balls){
    for(int i = 0; i < SIZE; i++){
        balls->whiteballs[i] = (rand() % 69) + 1;
    }
    balls->redball = (rand() % 26) + 1;
}

void print_balls(Balls balls){
    for(int i = 0; i < SIZE; i++){
        printf("%d ", balls.whiteballs[i]);
    }
    printf("%d\n", balls.redball);
}

int add_payouts(int payouts[]){
    int sum = 0;
    for(int i = 0; i < runs; i++){
        sum += payouts[i];
    }
    return sum;
}

void get_flags(int argc, char* argv[]){
    int opt;
    while((opt = getopt(argc, argv, "vf:dw::")) != -1){
        switch(opt){
            case 'v':
                flags += 1;
                break;
            case 'f':
                strcpy(file, optarg);
                if(strstr(file, ".txt") == NULL){
                    fprintf(stderr, "option: -f needs an argument\n");
                    free(file);
                    exit(EXIT_FAILURE);
                }
                flags += 2;
                break;
            case 'd':
                flags += 4;
                break;
            case 'w':
                (flags & VERBOSE) ? (flags += 7) : (flags += 8);
                if(optarg)
                    showWinningsAbove = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-vdwf] [file...] [runs]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if(optind < argc){
        //printf("%d\n", optind);
        runs = atoi(argv[optind]);
    }
}

void print_draw(Balls myBalls, Balls drawnBalls, int payout){
    printf("My balls: \t");
    print_balls(myBalls);
    printf("Drawn balls: \t");
    print_balls(drawnBalls);
    printf("Winnings: $%d\n", payout);
    printf("-------------------------------\n");
}

void print_to_file(char filename[], int total){
    int filefd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0744);
    char totalString[10];
    sprintf(totalString, "%d\n", total);
    int written = write(filefd, totalString, strlen(totalString));
    if(flags & DEV){
        printf("Bytes written: %d\n", written);
    }
    close(filefd);
}

int main(int argc, char* argv[]){

    //make it so that argv[1] can be --help and display how to use program

    //Handling flags
    get_flags(argc, argv);
    if(flags & DEV)
        printf("flags: %d %d %d %d\n", flags & VERBOSE, flags & TEXTFILE, flags & DEV, flags & WINNINGS);
    //runs = atoi(argv[argc-1]);

    //Declaring variables
    Balls myBalls;
    Balls drawnBalls;
    int payouts[runs];

    time_t t;
    clock_t start, end;
    double cpu_time_used;

    srand((unsigned) time(&t));
    
    //Simulate each lottery drawing
    start = clock();
    for(int i = 0; i < runs; i++){
        generate_balls(&myBalls);
        generate_balls(&drawnBalls);
        payouts[i] = get_payout(myBalls, drawnBalls);
        if(flags & VERBOSE || (flags & WINNINGS && payouts[i] >= showWinningsAbove)){
            print_draw(myBalls, drawnBalls, payouts[i]);
        }
    }
    int total = add_payouts(payouts);
    int earnings = total - cost * runs;
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Total winnings: $%d\n", total);
    printf("Net earnings: $%d\n", earnings);
    
    //Write to file if -f flag is on
    if(flags & TEXTFILE){
        print_to_file(file, earnings);
    }
    //Write CPU time used if -d flag is on
    if(flags & DEV)
        printf("CPU time: %f\n", cpu_time_used);

    free(file);
    return 0;
}