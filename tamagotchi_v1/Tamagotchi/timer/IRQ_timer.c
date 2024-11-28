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

void TIMER0_IRQHandler (void)
{
  /* Match register 0 interrupt service routine */
	if (LPC_TIM0->IR & 01) 																			//250ms
	{
		deleteCharacter1();
		deleteCharacter2();
		LPC_TIM0->IR = 1;			/* clear interrupt flag */
	}
		/* Match register 1 interrupt service routine */
	else if(LPC_TIM0->IR & 02)																	//500ms
  {
		deleteCharacter();
		GUI_Text(55, 150, "Your Tamagotchi", Black, White);
		GUI_Text(90, 170, "IS DEAD", Red, White);
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
	char str[20];
  /* Match register 0 interrupt service routine */
	if (LPC_TIM1->IR & 01)
	{
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
	else if(LPC_TIM2->IR & 4)
  {		
		LPC_TIM1->MR2 += 0x05f5e10;
	  if(LPC_TIM1->MR2 > 0x017D7840)
		{  
			 LPC_TIM1->MR2 = 0x05f5e10;
		}	
		
		LPC_TIM1->IR =  4 ;			/* clear interrupt flag */	
	}
		/* Match register 3 interrupt service routine */
	else if(LPC_TIM1->IR & 8)
  {
	 
		LPC_TIM1->IR =  8 ;			/* clear interrupt flag */	
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
	int leave = 0;
  /* Match register 0 interrupt service routine */
	if (LPC_TIM2->IR & 01)
	{
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
		
		if(leave) {
			endGame();
			/*disable_timer(1);
			disable_timer(2);
			LCD_DrawRectangle (0, 281, 240, 39, White);
			GUI_Text(100, 295, "Reset", Black, White);
			deleteCharacter();*/
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
		LPC_TIM2->MR2 += 0x05f5e10;
	  if(LPC_TIM2->MR2 > 0x017D7840)
		{  
			 LPC_TIM2->MR2 = 0x05f5e10;
		}	
		
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
  /* Match register 0 interrupt service routine */
	if (LPC_TIM3->IR & 01) 																			//250ms
	{
		deleteCharacter1();
		deleteCharacter2();
		deleteCharacter();
		
		moveDownLeft1();
	
		LPC_TIM3->IR = 1;			/* clear interrupt flag */
	}
		/* Match register 1 interrupt service routine */
	else if(LPC_TIM3->IR & 02)																	//500ms
  {
		deleteDownLeft1();
		moveDownLeft2();
		deleteMeal();
		deleteSnack();
		
		LPC_TIM3->IR =  2 ;			/* clear interrupt flag */	
	}
	/* Match register 2 interrupt service routine */
	else if(LPC_TIM3->IR & 4)																		//750ms
  {		
		deleteDownLeft2();
		moveDownLeft1();
		
		LPC_TIM3->IR =  4 ;			/* clear interrupt flag */	
	}
		/* Match register 3 interrupt service routine */
	else if(LPC_TIM3->IR & 8)																		//1s
  {
		deleteDownLeft1();
		buildCharacter();
		eating = 0;		//libero l'animazione e il joystick
		
		LPC_TIM3->IR =  8 ;			/* clear interrupt flag */	
	}
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
