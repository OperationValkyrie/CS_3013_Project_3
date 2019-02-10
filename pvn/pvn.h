/**
 * Jonathan CHang
 * CS 3013
 * Project 3
 */

#ifndef PVN_H_
#define PVN_H__

#define EXSEC 1
#define ADVENTURING 0
#define WAITING 1
#define SERVING 2
#define PIRATE 0
#define NINJA 1

struct visit {
    unsigned long long waitingStart;
    int waitingTime;
    int serviceTime;
    int costumeTeam;
};

struct costumeTeam {
    int state;
    int custNum;
    int totalServiceTime;
};

struct customer {
    int type;
    char title[10];
    int cTime;
    int aTime;
    int state;
    int numVisit;
    int ticket;
    struct visit visits[10];
};

struct costumeTeam costumeTeams[4];
struct customer customers[100];

int numCt;
int numPirate;
int numNinja;
int cTimePirate;
int cTimeNinja;
int aTimePirate;
int aTimeNinja;

int flag;
int currentTicket;
unsigned long long globalStart;
int totalGlobalTime;
int queueLength;
int totalQueueLength;
int queueMeasurements;

pthread_t thread[100];
sem_t sema;
sem_t semas[100];

void *customer(void *ptr);

void printStatistics();
unsigned long long getCurrentMilli();

int ifWaiting(int type);
int nextInLine();
int getTicketCustomer(int ticket);
int allFreeCostumeTeam();
int freeCostumeTeam();

int getNormal(int average);
int readArguments(int argc, char **argv);
void printArguments();
void printhelp();
#endif