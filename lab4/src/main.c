#include <cmsis_os2.h>
#include <LPC17xx.h>
#include <stdio.h>
#include <math.h>
#include "random.h"

struct Queue {
	osMessageQueueId_t qID;
	int failed;
	int totalSent;
	int totalReceived;
	float serviceTime;
};

float timeElapsed = 0, serviceTime = 0;

const int numQueues = 1;
struct Queue arrQueues[numQueues];

__NO_RETURN void displayMonitor(void * args) {
	while(1) {
		if ((int)timeElapsed % 20 == 0) {
				printf("ID, %10s, %10s, %10s, %10s, %10s, %10s, %10s, %10s, %10s, %10s, %10s\n", "Time", "Sent", "Received", 
			"Overflowed", "In Queue", "Pblk", "Arrv/sec", "Serv/sec", "Epblk", "Earrv", "Eserv");
			}
		for (int i = 0; i<numQueues; i++) {
			
			float Pblk = (float)arrQueues[i].failed / arrQueues[i].totalSent;
			float ArrvSec = arrQueues[i].totalSent / timeElapsed;
			float ServSec = arrQueues[i].totalReceived / arrQueues[i].serviceTime;
			float probFull = pow(9.0/10.0, 10) * ((1 - 9.0/10.0) / (1 - pow(9.0/10.0, 10+1)));
			
			printf("Q%d, %10d, %10d, %10d, %10d, %10d, %10f, ", i, (int)timeElapsed, arrQueues[i].totalSent, arrQueues[i].totalReceived, 
			arrQueues[i].failed, osMessageQueueGetCount(arrQueues[i].qID), Pblk);
			printf("%10f, %10f, %10f, %10f, %10f\n", ArrvSec, ServSec, (Pblk - probFull) / probFull, (ArrvSec - 9.0) / 9.0, (ServSec - 10.0) / 10.0);
		}
		timeElapsed += 1;
		osDelay(osKernelGetTickFreq());
	}
}

__NO_RETURN void client(void * args) {
	struct Queue* qid = (struct Queue*)(args);
	int msg = 5;
	while(1) {
		
		float delay = (next_event()/9) / pow(2,16);
		osDelay(delay*osKernelGetTickFreq());
		
		if(osMessageQueuePut(qid->qID, &msg, 0, 0) == osErrorResource) {
			qid->failed += 1;
		}
		qid->totalSent += 1;
	}
}

__NO_RETURN void server(void * args) {
	struct Queue* qid = (struct Queue*)(args);
	int msg;
	while(1) {
		
		float delay = (next_event()/10) / pow(2,16);
		qid->serviceTime += delay;
		osDelay(delay*osKernelGetTickFreq());
		
		osMessageQueueGet(qid->qID, &msg, NULL, osWaitForever);
		qid->totalReceived += 1;
	}
}

__NO_RETURN void app_main(void * args) {
	
	osThreadNew(displayMonitor, NULL, NULL);
	
	for (int i = 0; i<numQueues; i++) {
		osThreadNew(client, &arrQueues[i], NULL);
		osThreadNew(server, &arrQueues[i], NULL);
	}
	for (;;);
}

int main(void) {
	osKernelInitialize();
	for (int i=0; i<numQueues; i++) {
		arrQueues[i].qID = osMessageQueueNew(10, sizeof(int), NULL);
	}
	
	osThreadNew(app_main, NULL, NULL);
	osKernelStart();
	for(;;);
}
