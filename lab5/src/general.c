
/**
 * @brief Your source code goes here
 * @note You need to complete this file. New functions may be added to this file.
 */

#include <cmsis_os2.h>
#include "general.h"
#include <string.h>
// add any #includes here


#include <stdio.h>
#include <assert.h>

// add any #defines here

// add global variables here
uint8_t n = 0, m, count = 0,REPORTER, COMMANDER, visited = 0;
uint8_t traitors[7];
osMessageQueueId_t messQueues[7];

osMutexId_t mutex, print_m;
osSemaphoreId_t sem1;
osSemaphoreId_t sem2;

/** Record parameters and set up any OS and other resources
  * needed by your general() and broadcast() functions.
  * nGeneral: number of generals
  * loyal: array representing loyalty of corresponding generals
  * reporter: general that will generate output
  * return true if setup successful and n > 3*m, false otherwise
  */
bool setup(uint8_t nGeneral, bool loyal[], uint8_t reporter) {
	
	//char msg[10] = "Hello";
	//uint8_t size = strlen(msg);
	//printf("%d\n", size);
	//msg[4] = 'A';
	//printf("%s\n", msg);
	
	
	
	
	sem1 = osSemaphoreNew(1, 0, NULL);
	sem2 = osSemaphoreNew(1, 1, NULL);
	mutex = osMutexNew(NULL);
	print_m = osMutexNew(NULL);
	
	m = 0;
	n = nGeneral;
	REPORTER = reporter;
	for (int i=0;i<n;i++) {
		if (!loyal[i]) {
			traitors[i] = 1;
			m++;
		}
	}
	//printf("n = %d, m = %d\n", n,m);
	if(n <= 3*m) {
		printf("n > 3m failed.");
		return false;
	}
	
	
	for (int i = 0; i<n; i++) {
		messQueues[i] = osMessageQueueNew(30, sizeof(char)*7, NULL);
	}
	
    return true;
}

void barrier(uint8_t id) {
	
	//osMutexAcquire(print_m, osWaitForever);
	//printf("The barrier has been activated by General %d\n", id);
	//osMutexRelease(print_m);
	
	osMutexAcquire(mutex, osWaitForever);
	count++;
	if (count == n-1) {
		osSemaphoreAcquire(sem2, osWaitForever);
		osSemaphoreRelease(sem1);
	}
	osMutexRelease(mutex);
	
	osSemaphoreAcquire(sem1, osWaitForever);
	osSemaphoreRelease(sem1);
	
	osMutexAcquire(mutex, osWaitForever);
	
	count--;
	if (count == 0) {
		osSemaphoreAcquire(sem1, osWaitForever);
		osSemaphoreRelease(sem2);
	}
	osMutexRelease(mutex);
	
	osSemaphoreAcquire(sem2, osWaitForever);
	osSemaphoreRelease(sem2);
	
	//osMutexAcquire(print_m, osWaitForever);
	//printf("General %d passed the barrier.\n", id);
	//osMutexRelease(print_m);
}


/** Delete any OS resources created by setup() and free any memory
  * dynamically allocated by setup().
  */
void cleanup(void) {
	for(int i = 0; i<n; i++) {
		osMessageQueueDelete(messQueues[i]);
		messQueues[i] = NULL;
	}
	osSemaphoreDelete(sem1);
	osSemaphoreDelete(sem2);
	osMutexDelete(mutex);
}


/** This function performs the initial broadcast to n-1 generals.
  * It should wait for the generals to finish before returning.
  * Note that the general sending the command does not participate
  * in the OM algorithm.
  * command: either 'A' or 'R'
  * sender: general sending the command to other n-1 generals
  */
void broadcast(char command, uint8_t commander) {
	
	COMMANDER = commander;
	
	for (int i = 0; i<n; i++) {
		
		
		if (i == COMMANDER)
			continue;
		
		
		
		if (traitors[COMMANDER] == 1) {
			char action[10];
			if (i % 2 == 0)
				sprintf(action, "%d:R", COMMANDER);
			else
				sprintf(action, "%d:A", COMMANDER);
			osMessageQueuePut(messQueues[i], action, 0, 0);
		}
		
		else {
			char Command[10];
			sprintf(Command, "%d:%c", COMMANDER, command); 
			osMessageQueuePut(messQueues[i], Command, 0 , 0);
		}
		
		
		//printf("%s, sent_to=%d\n", Command, i);
		
	}
	//while (visited != n) {}
		osDelay(200);
}

bool found(char msg[10], int val, int len) {
	for(int i=0; i<len; i++) {
		if (msg[i] == val+'0')
			return true;
	}
	return false;
}
void om(uint8_t m, char msg[10], uint8_t id) {
	char msg_recieved[10];
	char attack = 'A';
	char retreat = 'R';
	
	if (m == 0 && id == REPORTER) {
		osMutexAcquire(print_m, osWaitForever);
		printf("%s\n", msg);
		osMutexRelease(print_m);
		return;
	}
	
	else if(m == 0)
		return;
	
	else {
		for (int i = 0; i<n; i++) {
			char msg1[10];
			char gen[2];
			sprintf(gen, "%d:", i);
			
			if (i == COMMANDER || i == id || strstr(msg, gen) != NULL) {
				continue;
			}
			
			
			if (traitors[id] == 1 && id%2 == 0) {
				sprintf(msg1, "%d:%s", id, msg);
				for (int i =0; i<10; i++) {
					if (msg1[i] == '\0') {
						msg1[i-1] = retreat;
						break;
					}	
				}
			}
			else if (traitors[id] == 1 && id%2 == 1) {
				sprintf(msg1, "%d:%s", id, msg);
				for (int i =0; i<10; i++) {
					if (msg1[i] == '\0') {
						msg1[i-1] = attack;
						break;
					}
				}
			}
			else {
				sprintf(msg1, "%d:%s", id, msg);
			}
			
			osMessageQueuePut(messQueues[i], msg1, 0, 0);
		}
		
		barrier(id);
		
		uint8_t num_mes = osMessageQueueGetCount(messQueues[id]);
		
		
		
		for (int i = 0; i<num_mes; i++) {
			osMessageQueueGet(messQueues[id], msg_recieved, 0, 0);
			om(m-1, msg_recieved, id);
		}
	}
}

/** Generals are created before each test and deleted after each
  * test.  The function should wait for a value from broadcast()
  * and then use the OM algorithm to solve the Byzantine General's
  * Problem.  The general designated as reporter in setup()
  * should output the messages received in OM(0).
  * idPtr: pointer to general's id number which is in [0,n-1]
  */
void general(void *idPtr) {
    uint8_t id = *(uint8_t *)idPtr;
	char msg[10];
	
	if (id != COMMANDER ) {
		osMessageQueueGet(messQueues[id], msg, NULL, osWaitForever);
		//printf("%s, received_by=%d\n", msg, id);
		om(m, msg, id);
	}
	visited++;
}
