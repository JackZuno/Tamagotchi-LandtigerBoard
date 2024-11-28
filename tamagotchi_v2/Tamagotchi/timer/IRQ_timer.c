/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lpc17xx.h"
#include "../adc/adc.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
int battery = 0;		//counter to 5

extern double volume;

uint16_t SinTable[45] =                                       /*                      */
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

const int freqss[8]={2120,1890,1684,1592,1417,1263,1125,1062};

void TIMER0_IRQHandler (void)
{
  /* Match register 0 interrupt service routine */
	if (LPC_TIM0->IR & 01) 																			//250ms
	{

		LPC_TIM0->IR = 1;			/* clear interrupt flag */
	}
		/* Match register 1 interrupt service routine */
	else if(LPC_TIM0->IR & 02)																	//500ms
  {
		
		LPC_TIM0->IR =  2 ;			/* clear interrupt flag */	
	}
	/* Match register 2 interrupt service routine */
	else if(LPC_TIM0->IR & 4)																		//750ms
  {		
		
		LPC_TIM0->IR =  4 ;			/* clear interrupt flag */	
	}
		/* Match register 3 interrupt service routine */
	else if(LPC_TIM0->IR & 8)																		//1s
  {

		LPC_TIM0->IR =  8 ;			/* clear interrupt flag */	
	}
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
	int leave = 0;
	char str[20];
	
	deleteHeart2();
	
  /* Match register 0 interrupt service routine */
	if (LPC_TIM1->IR & 01)
	{
		battery++;
		
		if(eating == 0) {						//se non sta mangiando faccio l'animazione
			buildCharacterOption1();
		} 
		ss++;
		if(ss == 60) {
			mm++;
			ss = 00;
			if(mm == 60) {
				hh++;
				mm = 00;
			}
		}
		sprintf(str, "%02d:%02d:%02d", hh, mm, ss);
		GUI_Text(115, 5, str, Black, White);
		
		LPC_TIM1->IR = 1;			/* clear interrupt flag */
	}
		/* Match register 1 interrupt service routine */
	else if(LPC_TIM1->IR & 02)
  {
		battery++;
		
		if(eating == 0) {
			buildCharacterOption2();			//se non sta mangiando fa l'animazione
		}
			
		ss++;
		if(ss == 60) {
			mm++;
			ss = 00;
			if(mm == 60) {
				hh++;
				mm = 00;
			}
		}
		sprintf(str, "%02d:%02d:%02d", hh, mm, ss);
		GUI_Text(115, 5, str, Black, White);
		
		LPC_TIM1->IR =  2 ;			/* clear interrupt flag */	
	}
	/* Match register 2 interrupt service routine */
	else if(LPC_TIM1->IR & 4)
  {		
		
		LPC_TIM1->IR =  4 ;			/* clear interrupt flag */	
	}
		/* Match register 3 interrupt service routine */
	else if(LPC_TIM1->IR & 8)
  {
	 
		LPC_TIM1->IR =  8 ;			/* clear interrupt flag */	
	}
	
	if(battery == 5) {
		switch(nHappiness) {
			case 1:
				LCD_DrawRectangle (45, 47, 6, 12, White);	//Happiness 1
				nHappiness--;
				//Dead
				leave = 1;
				break;
			case 2:
				LCD_DrawRectangle (53, 47, 6, 12, White);	//Happiness 2
				nHappiness--;
				break;
			case 3:
				LCD_DrawRectangle (61, 47, 6, 12, White);		//Happiness 3
				nHappiness--;
				break;
			case 4:
				LCD_DrawRectangle (69, 47, 6, 12, White);		//Happiness 4
				nHappiness--;
				break;
			default:
				break;
		}
		
		switch(nSatiety) {
			case 1:
				LCD_DrawRectangle (165, 47, 6, 12, White);		//Satiety 1
				nSatiety--;
				//dead
				leave = 1;
				break;
			case 2:
				LCD_DrawRectangle (173, 47, 6, 12, White);		//Satiety 2
				nSatiety--;
				break;
			case 3:
				LCD_DrawRectangle (181, 47, 6, 12, White);		//Satiety 3
				nSatiety--;
				break;
			case 4:
				LCD_DrawRectangle (189, 47, 6, 12, White);		//Satiety 4
				nSatiety--;
				break;
			default:
				break;
		}
		
		battery = 0;
		
		if(leave) {
			dead = 1;
			endGame();
			/*disable_timer(1);
			disable_timer(2);
			LCD_DrawRectangle (0, 281, 240, 39, White);
			GUI_Text(100, 295, "Reset", Black, White);
			deleteCharacter();*/
		}
	}
  return;
}

/******************************************************************************
** Function name:		Timer2_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER2_IRQHandler (void)
{
	static int ticks = 0;
	static int count = 0;
	static double val;
	
  /* Match register 0 interrupt service routine */
	if (LPC_TIM2->IR & 01)
	{
		/* DAC management */	
		val = SinTable[ticks]<<6;
		LPC_DAC->DACR = val*volume;
		ticks++;
		if(ticks==45 && count < 70) {
			ticks=0;
			count++;
		} else if(count == 70 && ticks == 45) {
			ticks = 0;
			count = 0;
			disable_timer(2);
		}
		
		LPC_TIM2->IR = 1;			/* clear interrupt flag */
	}
		/* Match register 1 interrupt service routine */
	else if(LPC_TIM2->IR & 02)
  {
		
		LPC_TIM2->IR =  2 ;			/* clear interrupt flag */	
	}
	/* Match register 2 interrupt service routine */
	else if(LPC_TIM2->IR & 4)
  {		
		
		LPC_TIM2->IR =  4 ;			/* clear interrupt flag */	
	}
		/* Match register 3 interrupt service routine */
	else if(LPC_TIM2->IR & 8)
  {
	 
		LPC_TIM2->IR =  8 ;			/* clear interrupt flag */	
	}
  return;
}

/******************************************************************************
** Function name:		Timer3_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER3_IRQHandler (void)
{
	static int countExit = 0;
	static int countEeat = 0;
	static int countCudd = 0;
	
  /* Match register 0 interrupt service routine */
	if (LPC_TIM3->IR & 01) 																			//250ms
	{
		if(eating) {
			switch(countEeat) {
				case 0:
					countEeat++;
					deleteCharacter1();
					deleteCharacter2();
					deleteCharacter();
					
					moveDownLeft1();
					
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[3]);
					enable_timer(2);
					break;
				
				case 1:
					countEeat++;
					deleteDownLeft1();
					moveDownLeft2();
					deleteMeal();
					deleteSnack();
					
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[3]);
					enable_timer(2);
					break;
				
				case 2:
					countEeat++;
					deleteDownLeft2();
					moveDownLeft1();
					
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[4]);
					enable_timer(2);
					break;
				
				case 3:
					countEeat=0;
					deleteDownLeft1();
					buildCharacter();
					
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[5]);
					enable_timer(2);
				
					eating = 0;		//libero l'animazione e il joystick
				
					disable_timer(3);
					break;
			}
		}
		
		if(dead) {
			switch(countExit) {
				case 0:
					countExit++;
					deleteCharacter1();
					deleteCharacter2();
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[7]);
					enable_timer(2);
				
					break;
				case 1:
					countExit++;
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[5]);
					enable_timer(2);
				
					break;
				case 2:
					countExit++;
					deleteCharacter();
					deleteCharacter1();
					deleteCharacter2();
					GUI_Text(55, 150, "Your Tamagotchi", Black, White);
					
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[2]);
					enable_timer(2);
				
					break;
				case 3:
					countExit++;
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[4]);
					enable_timer(2);
				
					break;
				case 4:
					countExit = 0;
					
					GUI_Text(90, 170, "IS DEAD", Red, White);
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[0]);
					enable_timer(2);
				
					disable_timer(3);
					break;
			}
		}
		
		if(cuddle) {
			switch(countCudd) {
				case 0:		//250ms
					countCudd++;
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[0]);
					enable_timer(2);
				
					break;
				case 1:		//500ms
					countCudd++;
				
					heart1();
					buildCharacterCuddle1();
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[1]);
					enable_timer(2);
				
					break;
				case 2:		//750ms
					countCudd++;
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[2]);
					enable_timer(2);
				
					break;
				case 3:		//1s
					countCudd++;
				
					deleteHeart1();
					heart2();
					buildCharacterCuddle2();
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[3]);
					enable_timer(2);
				
					break;
				case 4:		//1.25s
					countCudd++;
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[4]);
					enable_timer(2);
				
					break;
				case 5:		//1.5s
					countCudd++;
				
					deleteHeart2();
					heart1();
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[5]);
					enable_timer(2);
					break;
				case 6:		//1.75s
					countCudd++;
				
					deleteHeart1();
					heart2();
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[6]);
					enable_timer(2);
					break;
				case 7:		//2s
					countCudd = 0;		//cuddle finite
					cuddle = 0;
				
					reset_timer(2);
					init_timer(2, 0, 0, 3, freqss[7]);
					enable_timer(2);
				
					disable_timer(3);
					break;
			}
		}
		
		LPC_TIM3->IR = 1;			/* clear interrupt flag */
	}
		/* Match register 1 interrupt service routine */
	else if(LPC_TIM3->IR & 02)																	//500ms
  {
		
		LPC_TIM3->IR =  2 ;			/* clear interrupt flag */	
	}
	/* Match register 2 interrupt service routine */
	else if(LPC_TIM3->IR & 4)																		//750ms
  {		
		
		LPC_TIM3->IR =  4 ;			/* clear interrupt flag */	
	}
		/* Match register 3 interrupt service routine */
	else if(LPC_TIM3->IR & 8)																		//1s
  {
		
		LPC_TIM3->IR =  8 ;			/* clear interrupt flag */	
	}
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
