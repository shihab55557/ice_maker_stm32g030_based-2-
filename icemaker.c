#include "stm32g030xx.h"
#include "icemaker.h"
#include "serial.h"
#include "ntc.h"
/////// Steps for driving the ice maker ////////
/*
A.configure the motor pins 
	1.1 configure a function for MOTOR1 CW rotation
			P1(+), P2(GND)
	1.2 configure another function for MOTOR1 CCW rotation
			P1(GND), P2(+)
	1.3 Configure another function for MOTOR1 STOP operation
	

check whether icebox and ice container is empty if((IB=0) and (IC=0))

B. Fill water in ice box
	2. Turn ON the valve
	   2.1 if(vtime >= 6)
			 2.1.2 turn OFF the valve
			 2.1.3 Make the VALVE_FILLED_FLAG = 1;
			 2.1.4 Make vtime = 0;
		 2.2 else if
				VALVE_FILLED_FLAG = 0;


else if (IB=1) and (IC=1), check whether the ice container is empty or not

C. Check whether ice container empty or not checking hall feedback
	2.1 rotate ice maker CW
		if (4>time>=2)
		if (hall = 1)                                // ice contatiner still full
			2.1.1 set VALVE_FILLED_FLAG = 0            // so that it can't enter in condition using VALVE_FILLED_FLAG that makes (ICE_OPERATION_FLAG = 1)
			2.1.2 set ICE_OPERATION flag = 0           // THAT means it will not entered in the motor rotation loop 
		                                                and stay same as it is BY KEEPING ALL PARAMETERS SAME
				
		else if (hall = 0)                           // ice container empty
			2.2.1 set VALVE_FILLED_FLAG = 1            // enter in the condition using VALVE_FILLED_FLAG and check whether the ice is ready or not by checking temp
			// 2.2.2 set ICE_OPERATION flag = 1           // that means it will 
		rotate CCW
		
		if(time>=4)
			2.3.1 STOP the motor
				use a bloking delay of 3 minutes
		



check if (VALVE_FILLED_FLAG = 1) and 

C.Check whether ice is ready or not
	2. check the ice
	2.1 if(temp < -15)      // Ice is made; should be dispensed
		2.1.1 make the ICE_OPERATION flag =1
	2.2 else if(temp >= -15)
		2.2.1 make the ICE_OPERATION flag =0
		
		
	
	
Check whether icebox made ice if(ICE_OPERATION flag =1)

D.Rotate the motor
	2 Run the motor (CW initially)
			2.1 if(time>=2)         
					2.1.1 if(hall = 0)                	// check for hall feedback, Write a function for this , should be written 
																								 outside with a certain time interval (i.e from 2 to 5 sec)
						Run the motor (CW as before)      // run normally
					2.1.1.1 if (time>=7)
						STOP the motor 
					
					2.1.1.2 else if (time >= 9)
						Rotate the motor CCW
					
					2.1.1.3 else if (time >= 16)
						Stop the motor 
						MAKE THE COUNT FLAG = 0 (time = 0)
						MAKE THE ICE_OPERATION FLAG = 0
						MAKE IB = 0
						MAKE IC = 0
						VALVE_FILLED_FLAG = 0
					
				2.1.2 else if (hall = 1)
					Run the motor CCW
					
					2.1.2.1 if (time >= 4)
						Stop the motor 
						MAKE THE COUNT FLAG = 0 (time = 0)
						MAKE THE ICE_OPERATION FLAG = 0
						MAKE IC = 1;
						MAKE IB = 1;
						VALVE_FILLED_FLAG = 1
					
					
*/





volatile int16_t timer_value = -7;

// All the variable for counting time
volatile int16_t rtime = 0;      // To count the rotation time of motor
volatile int16_t vtime = 0;      // To count the valve on time


// All the flag for checking before an operation 
int IB = 0;                      // To update ice box status
int IC = 0;                      // To update ice container status
int VALVE_FILLED_FLAG = 0;       // To update whether valve filled water and ready to dispense ice
int ICE_OPERATION = 0;           // To update whether the system will go to ice dispense operation by rotating the motor



void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
			TIM1->SR &= ~TIM_SR_UIF;
			uart_send_char(" rtime = ");
			uart_send_num(rtime);
			
				vtime++;
			
			if(ICE_OPERATION == 1){
				rtime++;
			}
			else if(ICE_OPERATION == 0){
				if (rtime > 20){
					rtime = 0;
				}
			}
    }
}



void motor_control(int dir){            	// dir= direction of rotation
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;    	//GPIOA clock En
	GPIOA->MODER  &=~ GPIO_MODER_MODE2_1; 
	GPIOA->MODER  |= GPIO_MODER_MODE2_0;  	//make PA2 as GPIO output
	GPIOA->MODER  &=~ GPIO_MODER_MODE3_1; 
	GPIOA->MODER  |= GPIO_MODER_MODE3_0;  	//make PA3 as GPIO output
	GPIOA->MODER  &=~ GPIO_MODER_MODE4_1; 
	GPIOA->MODER  |= GPIO_MODER_MODE4_0;  	//make PA4 as GPIO output
	GPIOA->MODER  &=~ GPIO_MODER_MODE5_1; 
	GPIOA->MODER  |= GPIO_MODER_MODE5_0;  	//make PA5 as GPIO output
	
	if(dir == 1){              // M1 forward
		GPIOA -> ODR |= (1<<4);
		GPIOA ->BSRR |= (1<<4);  // PA2 high
		
		GPIOA -> ODR &=~ (1<<5);
		GPIOA ->BSRR &=~ (1<<5); // PA3  low
		uart_send_char("  M_F  ");
	}
	
	else if(dir == 2){         // M1 reverse
		GPIOA -> ODR &=~ (1<<4);
		GPIOA ->BSRR &=~ (1<<4); // PA2 low
		
		GPIOA -> ODR |= (1<<5);
		GPIOA ->BSRR |= (1<<5);  // PA3 high
		uart_send_char("  M_R  ");
	}
	
	else if (dir == 3){        // M1 stop
		GPIOA -> ODR &=~ (1<<4);
		GPIOA ->BSRR &=~ (1<<4); // PA2 low
		
		GPIOA -> ODR &=~ (1<<5);
		GPIOA ->BSRR &=~ (1<<5); // PA3  low
		uart_send_char("  M_S  ");
	}
}


int t1 = 0;
int t2 = 0;
int hall_f = 0;
int hall_feedback(void){
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;    	//GPIOB clock En
	GPIOB->MODER  &=~ GPIO_MODER_MODE0_0;
	GPIOB->MODER  &=~ GPIO_MODER_MODE0_1; //make PB0 as GPIO input

	if ((rtime>=2)&&(rtime<=3)){
		
		if (!(GPIOB->IDR & 0b01)){   // At lower state
			t1 = 1;
			//uart_send_char("got low");
		}
		if (t1 == 1){
			if ((GPIOB->IDR & 0b01)){  // At higher state
				t2 = 1;
				//uart_send_char("got high");
			}
		}
		if ((t1 == 1)&&(t2 == 1)){   // Condition for rising edge 
			hall_f = 1;                // Make hall feedback = 1
			//uart_send_char("make hall_f=1 ");
			t1 = 0;
			t2 = 0;
		}
	}
	return hall_f;
}






void valve_config(void){
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;    	//GPIOB clock En
	GPIOB->MODER  &=~ GPIO_MODER_MODE7_1; 
	GPIOB->MODER  |= GPIO_MODER_MODE7_0;  	//make PB7 as GPIO output
	GPIOB->OTYPER  &=~ GPIO_OTYPER_OT7;
	GPIOB -> ODR |= (1<<7);
	//GPIOB ->BSRR |= (1<<7);                 // PB7 high for turning valve on
	if((vtime > 7)){
		GPIOB -> ODR &=~ (1<<7);
		//GPIOB ->BSRR &=~ (1<<7);               // PB7 low for turning valve low
		VALVE_FILLED_FLAG = 1;
		
		uart_send_char(" vfilled ");
	}
	else {
		VALVE_FILLED_FLAG = 0;
		uart_send_char(" !vfilled ");
	}
}



void ice_box_rotation(void){
	uart_send_num(hall_feedback());
	uart_send_char("\n\r");
	
	if ((rtime >= 0)&&(rtime <= 3)){
		motor_control(1);
	}
	if((hall_feedback() == 0)){
		if((rtime > 3)&&(rtime <= 8)){
			motor_control(1);             // Motor1 rotating forward
		}
		if((rtime>8)&&(rtime<=10)){
			motor_control(3);             // Motor1 stop
		}
		else if((rtime>10)&&(rtime<=18)){
			motor_control(2);             // Motor1 rotating reverse
		}
		else if((rtime>18)&&(rtime<=20)){
			motor_control(3);             // Motor1 stop
		}
		else if(rtime > 20){			
			// Reset all the required flag and var
			rtime = 0;
			hall_f = 0;
			
			IB = 0;  // Ice box is full
			IC = 0;  // Ice container is full
			ICE_OPERATION = 0;
			VALVE_FILLED_FLAG = 0;
			vtime = 0;
		}
	}
	else if((hall_feedback() == 1)){     // Hall feedback = 1, that means motor should be run in reverse
		uart_send_char(" Hall=1 ");
		if ((rtime > 3)&&(rtime <= 5)){
			motor_control(3);            // Turn motor off for 2 seconds
		}
		else if((rtime > 5) && (rtime <=9)){
			motor_control(2);            // Rotate the motor in reverse for 3 seconds
		}
		else if((rtime > 9) && (rtime <= 11)){
			motor_control(3);            // Turn motor off and keep it for 2 seconds 
		}
		else if(rtime > 11){
			//Reset all required flag and var
			rtime = 0;
			hall_f = 0;
			IB = 1; // Ice box is full
			IC = 1; // Ice container is full
			ICE_OPERATION = 0;
			VALVE_FILLED_FLAG = 1;
			vtime = 0;
		}
	}
}




// Check whether ice is ready or not

void ice_temp_check(void){
	if(temperature() >= 250){
		ICE_OPERATION = 1;
	}
	else if(temperature() < 250 ){
		//ICE_OPERATION = 0;
	}
}



void ice_making_operation(void){
	

	uart_send_char("   HALL FEEDBACK   ");
	uart_send_num(hall_f);
	uart_send_char("    ");
	
	if((IB == 0)&&(IC == 0)){     // Step-1
			valve_config();
			uart_send_char("vconf");
			uart_send_char("\n\r");
	}
	
	if(VALVE_FILLED_FLAG == 1){  // Step-2
		ice_temp_check();
		uart_send_char(" tcheck ");
		uart_send_num(temperature());
		uart_send_char("\n\r");
	}
	
	
	if(ICE_OPERATION == 1){      // Step-3
		ice_box_rotation();
		uart_send_char(" ib rotat ");
		uart_send_char("\n\r");
	}

	//uart_send_char(" rtime= ");
	//uart_send_num(rtime);
	//uart_send_char(" vtime= ");
	//uart_send_num(vtime);
	//uart_send_char("\n\r");
	
}