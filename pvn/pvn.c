/**
 * Jonathan Chang
 * CS 3013
 * Project 3
 */

#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "pvn.h"

int main(int argc, char **argv) {
    if(readArguments(argc, argv) < 0) {
        printhelp();
        return -1;
    }
    printArguments();

    // Set up semaphores
    int i;
    int result = 0;
    for(i = 0; i < numPirate + numNinja; i++) {
        result = result | sem_init(&semas[i], 0, 0);
    }
    result = result | sem_init(&sema, 0, 1);
    if(result) {
        printf("%s\n", "Unable to initalize semaphores.");
        return -1;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);

    srand48(tv.tv_usec);
    flag = -1;
    currentTicket = 0;
    globalStart = getCurrentMilli();

    queueLength = 0;
    totalQueueLength = 0;
    queueMeasurements = 0;

    // Set up Sotume Teams
    for(i = 0; i < numCt; i++) {
        costumeTeams[i].state = WAITING;
        costumeTeams[i].totalServiceTime = 0;
    }
    // Set up pirates
    for(i = 0; i < numPirate; i++) {
        customers[i].type = PIRATE;
        customers[i].state = ADVENTURING;
        customers[i].aTime = aTimePirate;
        customers[i].cTime = cTimePirate;
        strcpy(customers[i].title, "Pirate");
        pthread_create(&thread[i], NULL, customer, (void *) i);
    }
    // Set up ninjas
    for(i = numPirate; i < numPirate + numNinja; i++) {
        customers[i].type = NINJA;
        customers[i].state = ADVENTURING;
        customers[i].aTime = aTimeNinja;
        customers[i].cTime = cTimeNinja;
        strcpy(customers[i].title, "Ninja");
        pthread_create(&thread[i], NULL, customer, (void *) i);
    }
    for(i = 0; i < numPirate + numNinja; i++) {
        pthread_join(thread[i], NULL);
    }
    totalGlobalTime = (int) ((getCurrentMilli() - globalStart) / 1000);
    printStatistics();
}

/**
 * Handles the entire run of a customer
 * @param ptr The pointer to the index of this customer
 */
void *customer(void *ptr) {
    int custNum = (int) ptr;
    int done = 0;

    while(!done) {
        sleep(getNormal(customers[custNum].aTime));

        printf("%s:%d %s\n", customers[custNum].title, custNum, "is now waiting.");

        sem_wait(&sema);
        customers[custNum].state = WAITING;
        customers[custNum].ticket = currentTicket++;
        customers[custNum].visits[customers[custNum].numVisit].waitingStart = getCurrentMilli();
        //Queue Statisitcs
        queueLength++;
        totalQueueLength += queueLength;
        queueMeasurements++;
        sem_post(&sema);

        // Waiting for service
        if(flag == -1) {
            sem_wait(&sema);
            flag = customers[custNum].type;
            sem_post(&sema);
        }
        // If my type being serviced
        if(flag == customers[custNum].type) {
            sem_wait(&sema);
            int result = freeCostumeTeam();
            // If other waiting then wait
            if(ifWaiting(!customers[custNum].type)) {
                result = -1;
            }
            if(result >= 0) {
                costumeTeams[result].state = SERVING;
                customers[custNum].visits[customers[custNum].numVisit].costumeTeam = result;
                customers[custNum].state = SERVING;
            }
            sem_post(&sema);

            if(result == -1) {
                sem_wait(&semas[custNum]);
            }
        // If not my type,
        } else {
            sem_wait(&sema);
            if(allFreeCostumeTeam()) {
                int result = freeCostumeTeam();
                if(result >= 0) {
                    costumeTeams[result].state = SERVING;
                    customers[custNum].visits[customers[custNum].numVisit].costumeTeam = result;
                    customers[custNum].state = SERVING;
                    flag = customers[custNum].type;
                }
                sem_post(&sema);
            } else {
                sem_post(&sema);
                sem_wait(&semas[custNum]);
            }
        }
        
        sem_wait(&sema);
        queueLength--;
        customers[custNum].visits[customers[custNum].numVisit].waitingTime = (int) ((getCurrentMilli() -
            customers[custNum].visits[customers[custNum].numVisit].waitingStart)/1000);
        int serviceTime = getNormal(customers[custNum].cTime);
        customers[custNum].visits[customers[custNum].numVisit].serviceTime = serviceTime;
        printf("%s:%d %s:%d %s:%d\n", customers[custNum].title, custNum, "is now being serviced by costume team", customers[custNum].visits[customers[custNum].numVisit].costumeTeam, "for", serviceTime);
        sem_post(&sema);

        sleep(serviceTime);

        // After service
        sem_wait(&sema);
        customers[custNum].state = ADVENTURING;
        costumeTeams[customers[custNum].visits[customers[custNum].numVisit].costumeTeam].state = WAITING;
        customers[custNum].numVisit++;

        int next = nextInLine();
        // If there are other customers in line
        if(next >= 0) {
            int waking = -1;
            // If the next person is my type, wake them up and let them in
            if(customers[next].type == customers[custNum].type) {
                waking = customers[custNum].type;
            // if next person is not my type and nobody else in, wake them up
            } else if(customers[next].type == !customers[custNum].type && allFreeCostumeTeam()) {
                waking = customers[next].type;
                flag = waking;
            }
            // else if next person is ot my type but people still in wake nobody up

            // Wake up based on ticket number
            int result = freeCostumeTeam();
            int i = next;
            while(result >= 0 && i >= 0) {
                if(customers[i].type == waking && customers[i].state == WAITING) {
                    costumeTeams[result].state = SERVING;
                    customers[i].state = SERVING;
                    customers[i].visits[customers[i].numVisit].costumeTeam = result;
                    printf("%s:%d %s %s:%d\n", customers[custNum].title, custNum, "wakes up", customers[i].title, i);
                    sem_post(&semas[i]);
                }
                i = getTicketCustomer(customers[i].ticket + 1);
                result = freeCostumeTeam();
            }
        }

        printf("%s:%d %s\n", customers[custNum].title, custNum, "is now adventuring.");
        sem_post(&sema);

        if(drand48() > 0.25) {
            done = 1;
        }
    }
    return 0;
}

/**
 * Calculates and prints out the statistics of the program
 */
void printStatistics() {
    int i;
    int totalGold = 0;
    int totalVisits = 0;
    printf("%s\n", "== == == == == == == ==");
    // For each customer print out informaltion of each visit
    for(i = 0; i < numPirate + numNinja; i++) {
        printf("%s:%d %s %d %s\n", customers[i].title, i, "visted", customers[i].numVisit, "times");
        int j;
        int totalServiceTime = 0;
        int free = 0;
        for(j = 0; j < customers[i].numVisit; j++) {
            struct visit v = customers[i].visits[j];
            printf("  %s:%d\n", "Visit", j);
            printf("    %-20s:%d\n", "Waiting Time", v.waitingTime);
            printf("    %-20s:%d\n", "Service Time", v.serviceTime);
            
            // Compute costume team statistics
            totalServiceTime += v.serviceTime;
            costumeTeams[v.costumeTeam].totalServiceTime += v.serviceTime;
            totalVisits++;
            if(v.waitingTime > 30) {
                free = 1;
            }
        }
        if(free) {
            totalServiceTime = 0;
        }
        totalGold += totalServiceTime;
        printf("  %-22s:%d\n", "Gold owned", totalServiceTime);
    }
    printf("%s:%d\n", "Total Gold Owned", totalGold);
    printf("%s\n", "== == == == == == == ==");
    // Print out costume team information
    for(i = 0; i < numCt; i++) {
        printf("%s:%d\n", "Costume Team", i);
        printf("  %-20s:%d\n", "Total Service Time", costumeTeams[i].totalServiceTime);
        printf("  %-20s:%d\n", "Total Waiting Time", totalGlobalTime - costumeTeams[i].totalServiceTime);
    }
    printf("%s\n", "== == == == == == == ==");
    // Overall statistics
    printf("%-20s:%d\n", "Total Revenue", totalGold);
    printf("%-20s:%f\n", "Gold per Visit", (totalGold / 1.0) / totalVisits);
    printf("%-20s:%f\n", "Average Queue Length", (totalQueueLength / 1.0) / queueMeasurements);
    printf("%-20s:%d\n", "Total Profit", totalGold - numCt * 5);
}

/**
 * Gets the next customer in line based on their ticket number
 * @returns The index of the next customer, (-1 is n customers in line)
 */
int nextInLine() {
    int i;
    int next = -1;
    int first = 1;
    for(i = 0; i < numPirate + numNinja; i++) {
        if(customers[i].state == WAITING && first) {
            next = i;
            first = 0;
        }
        if(customers[i].state == WAITING && customers[i].ticket < customers[next].ticket) {
            next = i;
        }
    }
    return next;
}

/**
 * Gets the customer with the given ticekt number
 * @param ticket The ticket nubmer ot look for
 * @returns The index of the customers (-1 if no ticket found)
 */
int getTicketCustomer(int ticket) {
    int i;
    for(i = 0; i < numPirate + numNinja; i++) {
        if(customers[i].ticket == ticket) {
            return i;
        }
    }
    return -1;
}

/**
 * Checks to see if any of the given type are waiting to be serviced
 * @param type The type to check
 * @return Whether or not that type is waiting (1 for waiting, 0 if none)
 */
int ifWaiting(int type) {
    int i;
    for(i = 0; i < numPirate + numNinja; i++) {
        if(customers[i].type == type && customers[i].state == WAITING) {
            return 1;
        }
    }
    return 0;
}

/**
 * Checks to see if all costume teams are free
 * @returns Whether all costume teams are free (1 for sucess, 0 otherwise)
 */
int allFreeCostumeTeam() {
    int i;
    for(i = 0; i < numCt; i++) {
        if(costumeTeams[i].state != WAITING) {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks to see if there is a free costume team,
 * if so return the number of the team
 * @returns The number of the free team, if none avaible return -1
 */
int freeCostumeTeam() {
    int i;
    for(i = 0; i < numCt; i++) {
        if(costumeTeams[i].state == WAITING) {
            return i;
        }
    }
    return -1;
}

/**
 * Gets a normal distribution of the given average using the 
 * Box-Muller Transformation. Set to always return a nubmer greater than 0
 * @param avearge The average to compute around
 * @returns the normal distribution number
 */
int getNormal(int average) {
    float a = drand48();
    float b = drand48();
    float c = fabs(sqrt(-2 * log(a)) * cos( 2 * acos(-1) * b));
    int result = (int) round(average * c);
    /*if(result <= 0) {
        result = 1;
    }*/
    return result;
}

/**
 * Gets the current time in milliseconds
 * @returns the current time in milliseconds
 */
unsigned long long getCurrentMilli() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long) tv.tv_sec * 1000 + 
            (unsigned long long) tv.tv_usec / 1000;
}

/**
 * Prints out the starting arguments for the program
 */
void printArguments() {
    printf("%s\n", "Running with arguments:");
    printf("  %-40s : %d\n", "Number of Pirates", numPirate);
    printf("    %-40s : %d\n", "Average Costuming Time for Pirate", cTimePirate);
    printf("    %-40s : %d\n", "Average Arrival Time of Pirate", aTimePirate);
    printf("  %-40s : %d\n", "Number of Ninjas", numNinja);
    printf("    %-40s : %d\n", "Average Costuming Time for Ninja", cTimeNinja);
    printf("    %-40s : %d\n", "Average Arrival Time of Ninja", aTimeNinja);
}

/**
 * Checks the given arguemnts for valid input
 * @param argc The number of arguemnts
 * @param argv The array of arguments
 * @returns Whether of not the arguemtns are valid (-1 for invalid, 0 for valid)
 */
int readArguments(int argc, char **argv) {
    if(argc != 8) {
        printf("%s\n", "Invalid nubmer of arguments");
        return -1;
    }

    numCt = atoi(argv[1]);
    if(!numCt || numCt < 2 || numCt > 4) {
        printf("%s\n", "Invalid number of costuming teams.");
        return -1;
    }

    numPirate = atoi(argv[2]);
    if(!numPirate || numPirate < 10 || numPirate > 50) {
        printf("%s\n", "Invalid number of pirates.");
        return -1;
    }

    numNinja = atoi(argv[3]);
    if(!numNinja || numNinja < 10 || numNinja > 50) {
        printf("%s\n", "Invalid number of ninjas.");
        return -1;
    }

    cTimePirate = atoi(argv[4]);
    if(!cTimePirate) {
        printf("%s\n", "Invalid average costume time for pirates.");
        return -1;
    }

    cTimeNinja = atoi(argv[5]);
    if(!cTimeNinja) {
        printf("%s\n", "Invalid average costume time for ninjas.");
        return -1;
    }

    aTimePirate = atoi(argv[6]);
    if(!aTimePirate) {
        printf("%s\n", "Invalid average arrival time for pirates.");
        return -1;
    }

    aTimeNinja = atoi(argv[7]);
    if(!aTimeNinja) {
        printf("%s\n", "Invalid avearge arrival time for ninjas.");
        return -1;
    }
    return 0;
}

/**
 * Prints out the help statement for the program
 */
void printhelp() {
    printf("%s\n", "This program required 7 arguments:");
    printf("%s\n", "  (int) number of costuming teams 2 <= n <= 4");
    printf("%s\n", "  (int) number of pirate 10 <= n <= 50");
    printf("%s\n", "  (int) number of ninjas 10 <= n <= 50");
    printf("%s\n", "  (int) average costume time of pirate in exsec");
    printf("%s\n", "  (int) average costume time of ninja in exsec");
    printf("%s\n", "  (int) average arrival time of pirate in exsec");
    printf("%s\n", "  (int) average arrival time of ninja in exsec");
}