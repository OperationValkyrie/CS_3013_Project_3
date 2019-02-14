#ifndef MDP_H_
#define MDP_H_

#define NUMCARS 20

#define RIGHT 1
#define STRAIGHT 2
#define LEFT 3

#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3
struct car {
    int num;
    int start;
    char dest[10];

    int x;
    int y;
    char pos[10];
    
    int turn;
    char turnChar[10];
    int currentDirection;
    int directions[3][2];
    char directionChar[3][10];
};

int whiteboard [2][2];

struct car cars[20];
pthread_t thread[20];
pthread_mutex_t wbmutex;
pthread_mutex_t lock;
pthread_mutex_t mutex[2][2];
pthread_cond_t cond[2][2];

//
int carQueue[4][20];

int currentCar[4];

int numCar[4];
//

void *car(void *ptr);

void signalAll();
int atLeastTwoFree();

void initalizeCars();

void printStart();
#endif