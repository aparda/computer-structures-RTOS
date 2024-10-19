/*----------------------------------------------------------------------------
* Name:    Zeyad Moustafa	zmoustaf@uwaterloo.ca
*	   Akbar Pardawalla	apardawa@uawterloo.ca
*
* Purpose: Implement functions for lab3
* Note(s):
*----------------------------------------------------------------------------*/

#include <cmsis_os2.h>
#include <LPC17xx.h>
#include <stdio.h>
#include <math.h>

void setup_led(){
	LPC_GPIO1->FIODIR = (1U << 28) | (1U << 29) | (1U << 31);
	LPC_GPIO1->FIOCLR = (1U << 28) | (1U << 29) | (1U << 31);
	
	LPC_GPIO2->FIODIR = (1U << 2) | (1U << 3) | (1U << 4) | (1U << 5) | (1U << 6);
	LPC_GPIO2->FIOCLR = (1U << 2) | (1U << 3) | (1U << 4) | (1U << 5) | (1U << 6);
}

//Function 1: Read the joystick
__NO_RETURN void read_joystick(void *args) {
	
	for (;;) {
		
		if ((LPC_GPIO1->FIOPIN & (1u << 23)) == 0) { //LED1
			printf("North - ");
			LPC_GPIO2->FIOSET |= (1U << 5);
			LPC_GPIO2->FIOCLR = (1U << 2) | (1U << 3) | (1U << 4) | (1U << 6);
		}
		
		else if ((LPC_GPIO1->FIOPIN & (1u << 24)) == 0) { //LED2
			printf("East - ");
			LPC_GPIO2->FIOSET |= (1U << 4);
			LPC_GPIO2->FIOCLR = (1U << 2) | (1U << 3) | (1U << 5) | (1U << 6);
		}
		
		else if ((LPC_GPIO1->FIOPIN & (1u << 25)) == 0) {//LED3
			printf("South - ");
			LPC_GPIO2->FIOSET |= (1U << 3);
			LPC_GPIO2->FIOCLR = (1U << 2) | (1U << 4) | (1U << 5) | (1U << 6);
		}
		
		else if ((LPC_GPIO1->FIOPIN & (1u << 26)) == 0) {//LED4
			printf("West - ");
			LPC_GPIO2->FIOSET |= (1U << 2);
			LPC_GPIO2->FIOCLR = (1U << 3) | (1U << 4) | (1U << 5) | (1U << 6);
		}
		
		else { //LED0
			printf("Center - ");
			LPC_GPIO2->FIOSET |= (1U << 6);
			LPC_GPIO2->FIOCLR = (1U << 2) | (1U << 3) | (1U << 4) | (1U << 5);
		}
		
		if ((LPC_GPIO1->FIOPIN & (1u << 20)) == 0) { //LED5 on
			printf("Pressed\n");
			LPC_GPIO1->FIOSET |= (1U << 31);
		}
		else {
			printf("Released\n"); //LED5 off
			LPC_GPIO1->FIOCLR = (1U << 31);
		}
		
		osDelay(50); //was 20
	}	
}

//Function 2: Read from the ADC and print out value in serial port
__NO_RETURN void print_adc (void *args) {
	
	const float range = 3.3 - 0;
	
	//Reset ADCR
	LPC_ADC->ADCR &= (0x0);
	//Setup
	//Turning the power on
	
	LPC_SC->PCONP |= (1U << 12); //Power up peripheral
	LPC_ADC->ADCR |= (1U << 2) | (1U << 8) | (1U << 21); //Choose ADC0.2; Clock; PDN Operational
	
	//Config the the function of Port 0 Pin 25
	LPC_PINCON->PINSEL1 |= (1U << 18);
	
	
	//Check voltage
	for (;;) {
		
		//Start Operation
		LPC_ADC->ADCR |= (1U << 24);
		
		//Check bit
		while ((LPC_ADC->ADGDR & (1U << 31)) == 0);
		
		//When input is available
		int input = (LPC_ADC->ADGDR >> 4U) & 0xfff;
		float result = input * range / (pow(2, 12) - 1);
		printf("Result %f\n", result);
		
		osDelay(50);
	}
}
	
//Function 3: Push Button pressed/released
__NO_RETURN void push_button (void *args) {
	
	for (;;){
		//off
		if ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0){
			//LPC_GPIO1->FIOSET = (1U << 28);
			
			//Waiting for button to not be pressed
			while ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0)
			//while ((LPC_GPIO2->FIOPIN & (1 << 10)) != 0);
			
			LPC_GPIO1->FIOSET = (1U << 28);
			
			while ((LPC_GPIO2->FIOPIN & (1 << 10)) != 0); 
			while ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0);
			LPC_GPIO1->FIOCLR = (1U << 28);
			
			osDelay(50);
		}
	}
}
	
__NO_RETURN void app_main(void *args) {
	
	osThreadNew(read_joystick, NULL, NULL);
	osThreadNew(print_adc, NULL, NULL);
	osThreadNew(push_button, NULL, NULL);
	
	for (;;);
	
}

int main (void) {
	
	SystemCoreClockUpdate();
	osKernelInitialize();
	setup_led();
	SystemInit();
	osThreadNew(app_main, NULL, NULL);
	osKernelStart();

	return 0;

}
