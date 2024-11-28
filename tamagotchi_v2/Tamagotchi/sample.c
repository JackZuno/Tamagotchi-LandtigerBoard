/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "adc/adc.h"
#include "button_EXINT/button.h"
#include "GLCD/GLCD.h" 
#include "joystick/joystick.h"
#include "led/led.h"
#include "RIT/RIT.h"
#include "timer/timer.h"
#include "TouchPanel/TouchPanel.h"

#define SIMULATOR 1

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


int main(void)
{
  SystemInit();  												/* System Initialization (i.e., PLL)  */
  LCD_Initialization();
	
	TP_Init();
	TouchPanel_Calibrate();		//se voglio usare il touch devo attivarlo
	
	BUTTON_init();	
	joystick_init();
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec       */
	enable_RIT();
	
	ADC_init();
	
	//Timer animazione personaggio e diminuzione delle batterie
	init_timer(1, 0, 0, 1, 0x017D7840);		//MR0		1s --> 0x017D7840	
	init_timer(1, 0, 1, 3, 0x02FAF080);   //MR1 		2s --> 0x02FAF080	
		
	//Timer animazione cibo (durata 1s)
	init_timer(3, 0, 0, 3, 0x005F5E10);		//MR0		250ms --> 0x005F5E10
	
	//Timer animazione endGame (durata 1s)
	init_timer(0, 0, 0, 1, 0x00BEBC20);		//MR0		500ms --> 0x00BEBC20	
	init_timer(0, 0, 1, 1, 0x017D7840);   //MR1 	1s 		--> 0x017D7840
	
	/*TIMER 3 (animazione per mangiare)
			-250ms --> 0x005F5E10 (prima transizione, mi sposto di una posizione)
			-500ms --> 0x00BEBC20	(seconda transizione, arrivo davanti al cibo)
			-750ms --> 0x011E1A30	(terza transizione, torno alla posizione 250ms)
			-1s 	 --> 0x017D7840 (torno alla stessa posizione in cui ero in partenza)
	*/
	
	LCD_Clear(White);
	
	enable_timer(1);		//timer animazione personaggio
		
	ageSet(); 					//age
	
	buildCharacter();		//creation character
	mealSnackSpace();		//creation space e text for meal and snack
	batteryDraw();			//battery slots
	drawVolume();
	
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);	

	//aggiunta per usare spotify
	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);
	
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/


//animazione pranzo
/*
	uso il timer 3 settato ad un secondo, con il MR0 che fa interrupt e reset, Il secondo del timer lo divido in diversi passi, 
	ognuno in un intervallo preciso in cui il personaggio si sposta verso il cibo.
	Durante questa animazione disabilito il joystic
*/
