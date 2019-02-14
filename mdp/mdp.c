/**
 * Jonathan Chang
 * CS 3013
 * Project 3
 */

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mdp.h"

int main(int argc, char **argv) {
    srand48(100);
    initalizeCars();
    printStart();

    pthread_mutex_init(&wbmutex, NULL);
    pthread_mutex_init(&lock, NULL);

    pthread_mutex_init(&mutex[0][0], NULL);
    pthread_mutex_init(&mutex[0][1], NULL);
    pthread_mutex_init(&mutex[1][0], NULL);
    pthread_mutex_init(&mutex[1][1], NULL);

    pthread_cond_init(&cond[0][0], NULL);
    pthread_cond_init(&cond[0][1], NULL);
    pthread_cond_init(&cond[1][0], NULL);
    pthread_cond_init(&cond[1][1], NULL);

    whiteboard[0][0] = 0;
    whiteboard[0][1] = 0;
    whiteboard[1][0] = 0;
    whiteboard[1][1] = 0;

    int i;
    for(i = 0; i < NUMCARS; i++) {
        pthread_create(&thread[i], NULL, car, (void *) i);
    }
    for(i = 0; i < NUMCARS; i++) {
        pthread_join(thread[i], NULL);
    }
    return 0;
}

void *car(void *ptr) {
    int carNum = (int) ptr;
    int carStart = cars[carNum].start;
    int i;

    for(i = 0; i < cars[carNum].turn; i++) {
        int targetX = cars[carNum].directions[i][0];
        int targetY = cars[carNum].directions[i][1];

        pthread_mutex_lock(&mutex[targetX][targetY]);

        while(i == 0 && (carQueue[carStart][currentCar[carStart]] != carNum || !atLeastTwoFree())) {
            pthread_cond_wait(&cond[targetX][targetY], &mutex[targetX][targetY]);
        }

        while(whiteboard[targetX][targetY] != 0) {
            pthread_cond_wait(&cond[targetX][targetY], &mutex[targetX][targetY]);
        }

        pthread_mutex_lock(&wbmutex);
        if(i != 0) {
            whiteboard[cars[carNum].x][cars[carNum].y] = 0;
        } else {
            currentCar[carStart]++;
        }
        whiteboard[targetX][targetY] = 1;
        pthread_mutex_unlock(&wbmutex);

        if(i != 0) {
            pthread_cond_broadcast(&cond[cars[carNum].x][cars[carNum].y]);
        }

        cars[carNum].x = targetX;
        cars[carNum].y = targetY;
        printf("%s:%2d %s %5s %s %5s\n", "Car", carNum, "moves from", 
            cars[carNum].pos, "to", cars[carNum].directionChar[i]);
        strcpy(cars[carNum].pos, cars[carNum].directionChar[i]);
        pthread_mutex_unlock(&mutex[targetX][targetY]);

        sleep(1);
    }
    pthread_mutex_lock(&wbmutex);
    whiteboard[cars[carNum].x][cars[carNum].y] = 0;
    pthread_mutex_unlock(&wbmutex);
    pthread_cond_broadcast(&cond[0][0]);
    pthread_cond_broadcast(&cond[0][1]);
    pthread_cond_broadcast(&cond[1][0]);
    pthread_cond_broadcast(&cond[1][1]);

    //pthread_cond_broadcast(&cond[cars[carNum].x][cars[carNum].y]);

    printf("%s:%2d %s %5s %s %5s\n", "Car", carNum, "moves from", 
        cars[carNum].pos, "to", cars[carNum].dest);
    return 0;
}

int atLeastTwoFree() {
    pthread_mutex_lock(&wbmutex);
    int sum = whiteboard[0][0];
    sum += whiteboard[0][1];
    sum += whiteboard[1][0];
    sum += whiteboard[1][1];
    pthread_mutex_unlock(&wbmutex);
    return sum <= 2;
}
void printStart() {
    int i;
    printf("%s\n", "North Cars:");
    for(i = 0; i < numCar[NORTH]; i++) {
        printf("  %s:%2d %s %s\n", "Car", cars[carQueue[NORTH][i]].num, "going", cars[carQueue[NORTH][i]].turnChar);
    }
    printf("%s\n", "South Cars:");
    for(i = 0; i < numCar[SOUTH]; i++) {
        printf("  %s:%2d %s %s\n", "Car", cars[carQueue[SOUTH][i]].num, "going", cars[carQueue[SOUTH][i]].turnChar);
    }
    printf("%s\n", "West Cars:");
    for(i = 0; i < numCar[WEST]; i++) {
        printf("  %s:%2d %s %s\n", "Car", cars[carQueue[WEST][i]].num, "going", cars[carQueue[WEST][i]].turnChar);
    }
    printf("%s\n", "East Cars:");
    for(i = 0; i < numCar[EAST]; i++) {
        printf("  %s:%2d %s %s\n", "Car", cars[carQueue[EAST][i]].num, "going", cars[carQueue[EAST][i]].turnChar);
    }
}

void initalizeCars() {
    int i;
    currentCar[NORTH] = 0;
    currentCar[SOUTH] = 0;
    currentCar[WEST] = 0;
    currentCar[EAST] = 0;

    numCar[NORTH] = 0;
    numCar[SOUTH] = 0;
    numCar[WEST] = 0;
    numCar[EAST] = 0;

    for(i = 0; i < NUMCARS; i++) {
        float a = drand48();
        cars[i].num = i;
        cars[i].currentDirection = 0;
        cars[i].x = -1;
        cars[i].y = -1;
        // Start North
        if(a < 0.25) {
            carQueue[NORTH][numCar[NORTH]++] = i;
            cars[i].start = NORTH;
            strcpy(cars[i].pos, "north");

            cars[i].directions[0][0] = 0;
            cars[i].directions[0][1] = 0;
            strcpy(cars[i].directionChar[0], "NW");

            float b = drand48();
            // Go Right
            if(b <= 1.0/3) {
                cars[i].turn = RIGHT;
                strcpy(cars[i].turnChar, "right");
                strcpy(cars[i].dest, "west");
            }
            // Go Straight
            if(b > 1.0/3) {
                cars[i].turn = STRAIGHT;
                strcpy(cars[i].turnChar, "straight");
                cars[i].directions[1][0] = 1;
                cars[i].directions[1][1] = 0;
                strcpy(cars[i].directionChar[1], "SW");
                strcpy(cars[i].dest, "south");
                // Go Left
                if(b > 2.0/3) {
                    cars[i].turn = LEFT;
                    strcpy(cars[i].turnChar, "left");
                    cars[i].directions[2][0] = 1;
                    cars[i].directions[2][1] = 1;
                    strcpy(cars[i].directionChar[2], "SE");
                    strcpy(cars[i].dest, "east");
                }
            }
        // Start South
        } else if(a < 0.50) {
            carQueue[SOUTH][numCar[SOUTH]++] = i;
            cars[i].start = SOUTH;
            strcpy(cars[i].pos, "south");

            cars[i].directions[0][0] = 1;
            cars[i].directions[0][1] = 1;
            strcpy(cars[i].directionChar[0], "SE");

            float b = drand48();
            // Go Right
            if(b <= 1.0/3) {
                cars[i].turn = RIGHT;
                strcpy(cars[i].turnChar, "right");
                strcpy(cars[i].dest, "east");
            }
            // Go Straight
            if(b > 1.0/3) {
                cars[i].turn = STRAIGHT;
                strcpy(cars[i].turnChar, "straight");
                cars[i].directions[1][0] = 0;
                cars[i].directions[1][1] = 1;
                strcpy(cars[i].directionChar[1], "NE");
                strcpy(cars[i].dest, "north");
                // Go Left
                if(b > 2.0/3) {
                    cars[i].turn = LEFT;
                    strcpy(cars[i].turnChar, "left");
                    cars[i].directions[2][0] = 0;
                    cars[i].directions[2][1] = 0;
                    strcpy(cars[i].directionChar[2], "NW");
                    strcpy(cars[i].dest, "west");
                }
            }
        // Start West
        } else if(a < 0.75) {
            carQueue[WEST][numCar[WEST]++] = i;
            cars[i].start = WEST;
            strcpy(cars[i].pos, "west");
            
            cars[i].directions[0][0] = 1;
            cars[i].directions[0][1] = 0;
            strcpy(cars[i].directionChar[0], "SW");

            float b = drand48();
            // Go Right
            if(b <= 1.0/3) {
                cars[i].turn = RIGHT;
                strcpy(cars[i].turnChar, "right");
                strcpy(cars[i].dest, "south");
            }
            // Go Straight
            if(b > 1.0/3) {
                cars[i].turn = STRAIGHT;
                strcpy(cars[i].turnChar, "straight");
                cars[i].directions[1][0] = 1;
                cars[i].directions[1][1] = 1;
                strcpy(cars[i].directionChar[1], "SE");
                strcpy(cars[i].dest, "east");
                // Go Left
                if(b > 2.0/3) {
                    cars[i].turn = LEFT;
                    strcpy(cars[i].turnChar, "left");
                    cars[i].directions[2][0] = 0;
                    cars[i].directions[2][1] = 1;
                    strcpy(cars[i].directionChar[2], "NE");
                    strcpy(cars[i].dest, "north");
                }
            }
        // Start East
        } else {
            carQueue[EAST][numCar[EAST]++] = i;
            cars[i].start = EAST;
            strcpy(cars[i].pos, "east");
            
            cars[i].directions[0][0] = 0;
            cars[i].directions[0][1] = 1;
            strcpy(cars[i].directionChar[0], "NE");

            float b = drand48();
            // Go Right
            if(b <= 1.0/3) {
                cars[i].turn = RIGHT;
                strcpy(cars[i].turnChar, "right");
                strcpy(cars[i].dest, "north");
            }
            // Go Straight
            if(b > 1.0/3) {
                cars[i].turn = STRAIGHT;
                strcpy(cars[i].turnChar, "straight");
                cars[i].directions[1][0] = 0;
                cars[i].directions[1][1] = 0;
                strcpy(cars[i].directionChar[1], "NW");
                strcpy(cars[i].dest, "west");
                // Go Left
                if(b > 2.0/3) {
                    cars[i].turn = LEFT;
                    strcpy(cars[i].turnChar, "left");
                    cars[i].directions[2][0] = 1;
                    cars[i].directions[2][1] = 0;
                    strcpy(cars[i].directionChar[2], "SW");
                    strcpy(cars[i].dest, "south");
                }
            }
        }
    }
}