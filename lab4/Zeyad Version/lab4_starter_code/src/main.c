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
};


/**
 * Table of -ln(x), x in [1/256, 2/256, ..., 256/256] in fixed point
 * representation.  Generated by make_table.c.
 */
static uint32_t ln_table[256] = { 
	363409, 317983, 291410, 272557, 257933, 245984, 235882, 227130, 219411, 
	212507, 206260, 200558, 195312, 190455, 185934, 181704, 177731, 173985, 
	170442, 167080, 163883, 160834, 157921, 155132, 152457, 149886, 147413, 
	145029, 142730, 140508, 138359, 136278, 134262, 132305, 130405, 128559, 
	126764, 125016, 123314, 121654, 120036, 118457, 116915, 115408, 113935, 
	112495, 111085, 109706, 108354, 107030, 105733, 104460, 103212, 101987, 
	100784,  99603,  98443,  97304,  96183,  95082,  93999,  92933,  91884, 
	 90852,  89836,  88836,  87850,  86879,  85922,  84979,  84050,  83133, 
	 82229,  81338,  80458,  79590,  78733,  77887,  77053,  76228,  75414, 
	 74610,  73816,  73031,  72255,  71489,  70731,  69982,  69241,  68509, 
	 67785,  67069,  66360,  65659,  64966,  64280,  63600,  62928,  62263, 
	 61604,  60952,  60307,  59667,  59034,  58407,  57786,  57170,  56561, 
	 55957,  55358,  54765,  54177,  53595,  53017,  52445,  51877,  51315, 
	 50757,  50204,  49656,  49112,  48572,  48037,  47507,  46980,  46458, 
	 45940,  45426,  44916,  44410,  43908,  43409,  42915,  42424,  41937, 
	 41453,  40973,  40496,  40023,  39553,  39087,  38624,  38164,  37707, 
	 37254,  36803,  36356,  35911,  35470,  35032,  34596,  34164,  33734, 
	 33307,  32883,  32461,  32043,  31627,  31213,  30802,  30394,  29988, 
	 29585,  29184,  28786,  28390,  27996,  27605,  27216,  26829,  26445, 
	 26063,  25683,  25305,  24929,  24556,  24185,  23815,  23448,  23083, 
	 22720,  22359,  22000,  21643,  21288,  20934,  20583,  20233,  19886, 
	 19540,  19196,  18854,  18513,  18174,  17837,  17502,  17169,  16837, 
	 16507,  16178,  15851,  15526,  15202,  14880,  14560,  14241,  13924, 
	 13608,  13294,  12981,  12669,  12360,  12051,  11744,  11439,  11135, 
	 10832,  10530,  10231,   9932,   9635,   9339,   9044,   8751,   8459, 
	  8169,   7879,   7591,   7304,   7019,   6734,   6451,   6169,   5889, 
	  5609,   5331,   5054,   4778,   4503,   4230,   3957,   3686,   3415, 
	  3146,   2878,   2611,   2345,   2081,   1817,   1554,   1293,   1032, 
	   773,    514,    257 };

const int numQueues = 1;
struct Queue arrQueues[numQueues];
	   
uint32_t next_event(void) {
	// Get a 32-bit unsigned random value.  Shift it right 24 bits to
	// get the 8 most-signficant bits.  Use those to index into the
	// 256-entry table of natural logs. 
	return ln_table[(uint8_t)(lfsr113() >> 24)];
}

__NO_RETURN void displayMonitor(struct Queue queues[]) {
	for (int i = 0)
	
}

__NO_RETURN void client(struct Queue qid) {
	int msg = 5;
	while(1) {
		
		unsigned int delay = (next_event()/9) / (1u << 16);
		osDelay(delay*osKernelGetTickFreq());
		
		if(osMessageQueuePut(qid.qID, &msg, 0, 0) == osErrorResource) {
			qid.failed += 1;
		}
		
		qid.totalSent += 1;
	}
}

__NO_RETURN void server(struct Queue qid) {
	int msg;
	while(1) {
		
		unsigned int delay = (next_event()/10) / (1u << 16);
		osDelay(delay*osKernelGetTickFreq());
		
		osMessageQueueGet(qid.qID, &msg, NULL, osWaitForever);
		qid.totalReceived += 1;
	}
}

__NO_RETURN void app_main(void) {
	osThreadNew(client, NULL, NULL);
	osThreadNew(server, NULL, NULL);
	osThreadNew(displayMonitor, NULL, NULL);
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

