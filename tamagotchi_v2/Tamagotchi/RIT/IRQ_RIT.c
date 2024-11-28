/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "../adc/adc.h"
#include "../GLCD/GLCD.h"
#include "RIT.h"
#include "../led/led.h"
#include "../TouchPanel/TouchPanel.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

volatile int selectedHap = 0;
volatile int selectedSat = 0;

const int freqs[8]={2120,1890,1684,1592,1417,1263,1125,1062};

void RIT_IRQHandler (void)
{					
	static int j_select = 0;	//25
	static int j_down = 0;		//26
	static int j_left = 0;		//27
	static int j_right = 0; 	//28
	static int j_up = 0;			//29
	
	/* ADC management */
	ADC_start_conversion();	
	
	//SELECT
	if((LPC_GPIO1->FIOPIN & (1<<25)) == 0){			
		j_select++;
		
		switch(j_select){
			case 1:
				//code
				if(selectedSat == 1 && nSatiety < 4) {
					eating = 1;
					createMeal();
					eat();
					updateSatiety();			//do il meal
					selectedSat = 0;
					GUI_Text(40, 295, "Meal", Black, White);
				}
				if(selectedHap == 1 && nHappiness < 4) {
					eating = 1;
					createSnack();
					eat();
					updateHappiness();     		//do lo snack
					selectedHap = 0;
					GUI_Text(160, 295, "Snack", Black, White);
				}
				if(getHap() == 0 || getSat() == 0) {
					restart();
				}
				break;
			default:
				break;
		}
	}
	else{
			j_select=0;
	}
	
	//DOWN
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){			
		j_down++;
		
		switch(j_down){
			case 1:
				//code
				break;
			default:
				break;
		}
	}
	else{
			j_down=0;
	}
	
	//LEFT
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){			
		j_left++;
		
		switch(j_left){
			case 1:
				//code
				if(selectedHap == 0 && eating == 0  && nHappiness>0 && nSatiety>0) {
						GUI_Text(40, 295, "Meal", Red, White);
					
						reset_timer(2);
						init_timer(2, 0, 0, 3, freqs[6]);
						enable_timer(2);
					
						selectedSat = 1;
					} else if(selectedHap && eating == 0  && nHappiness>0 && nSatiety>0) {
						GUI_Text(40, 295, "Meal", Red, White);
						selectedHap = 0;
						selectedSat = 1;
						
						reset_timer(2);
						init_timer(2, 0, 0, 3, freqs[6]);
						enable_timer(2);
						
						GUI_Text(160, 295, "Snack", Black, White);
				}
				break;
			default:
				break;
		}
	}
	else{
			j_left=0;
	}
	
	//RIGHT
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){			
		j_right++;
		
		switch(j_right){
			case 1:
				//code
				if(selectedSat == 0 && eating == 0 && nSatiety>0 && nHappiness>0) {
						GUI_Text(160, 295, "Snack", Red, White);
						
						reset_timer(2);
						init_timer(2, 0, 0, 3, freqs[7]);
						enable_timer(2);
					
						selectedHap = 1;
					} else if(selectedSat && eating == 0 && nSatiety>0 && nHappiness>0) {
						GUI_Text(160, 295, "Snack", Red, White);
						
						reset_timer(2);
						init_timer(2, 0, 0, 3, freqs[7]);
						enable_timer(2);
						
						selectedSat = 0;
						selectedHap = 1;
						GUI_Text(40, 295, "Meal", Black, White);
				}
				break;
			default:
				break;
		}
	}
	else{
			j_right=0;
	}
	
	//UP
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){			
		j_up++;
		
		switch(j_up){
			case 1:
				//code
				break;
			default:
				break;
		}
	}
	else{
			j_up=0;
	}
	
	/* button management */
	
	if(getDisplayPoint(&display, Read_Ads7846(), &matrix )){
			if(display.y >= 90 && display.y <= 230 && display.x >= 55 && display.x <= 185){
				if((!eating && !cuddle) && nHappiness>0 && nHappiness<4) {
					cuddle = 1;
					enable_timer(3);
				}
			}
		}
		else {
			//do nothing if touch returns values out of bounds
		}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
	disable_RIT();
	reset_RIT();
	enable_RIT();
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
