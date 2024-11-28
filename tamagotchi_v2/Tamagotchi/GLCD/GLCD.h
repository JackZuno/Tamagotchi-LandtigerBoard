/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.h
** Descriptions:		Has been tested SSD1289¡¢ILI9320¡¢R61505U¡¢SSD1298¡¢ST7781¡¢SPFD5408B¡¢ILI9325¡¢ILI9328¡¢
**						HX8346A¡¢HX8347A
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2012-3-10
** Version:				1.3
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

#ifndef __GLCD_H 
#define __GLCD_H

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "AsciiLib.h"
#include "../timer/timer.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t LCD_Code;
static uint16_t back_color;

/* LCD Interface */
#define PIN_EN		(1 << 19)
#define PIN_LE		(1 << 20)
#define PIN_DIR		(1 << 21)
#define PIN_CS      (1 << 22)
#define PIN_RS		(1 << 23)
#define PIN_WR		(1 << 24)
#define PIN_RD		(1 << 25)   

#define LCD_EN(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_EN) : (LPC_GPIO0->FIOCLR = PIN_EN));
#define LCD_LE(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_LE) : (LPC_GPIO0->FIOCLR = PIN_LE));
#define LCD_DIR(x)  ((x) ? (LPC_GPIO0->FIOSET = PIN_DIR) : (LPC_GPIO0->FIOCLR = PIN_DIR));
#define LCD_CS(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_CS) : (LPC_GPIO0->FIOCLR = PIN_CS));
#define LCD_RS(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_RS) : (LPC_GPIO0->FIOCLR = PIN_RS));
#define LCD_WR(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_WR) : (LPC_GPIO0->FIOCLR = PIN_WR));
#define LCD_RD(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_RD) : (LPC_GPIO0->FIOCLR = PIN_RD));

/* Private define ------------------------------------------------------------*/
#define DISP_ORIENTATION  0  /* angle 0 90 */ 

#if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )

#define  MAX_X  320
#define  MAX_Y  240   

#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )

#define  MAX_X  240
#define  MAX_Y  320   

#endif

/* LCD color */
#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

//global
extern int ss;
extern int mm;
extern int hh;

extern int nHappiness;
extern int nSatiety;

extern int eating;

extern int dead;

extern int cuddle;

/******************************************************************************
* Function Name  : RGB565CONVERT
* Description    : 24Î»×ª»»16Î»
* Input          : - red: R
*                  - green: G 
*				   - blue: B
* Output         : None
* Return         : RGB ÑÕÉ«Öµ
* Attention		 : None
*******************************************************************************/
#define RGB565CONVERT(red, green, blue)\
(uint16_t)( (( red   >> 3 ) << 11 ) | \
(( green >> 2 ) << 5  ) | \
( blue  >> 3 ))

/* Private function prototypes -----------------------------------------------*/
void LCD_Initialization(void);
void LCD_Clear(uint16_t Color);
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos);
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point);
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color );
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor );
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor);

/*	User function	protoyupes --------------------------------------------------*/
void drawShape(uint16_t lenght, uint16_t x0, uint16_t y0, uint16_t color);		//disegno il triangolo (corpo del personaggio)

void LCD_SetWindow (uint16_t X_start, uint16_t Y_start, uint16_t width, uint16_t height);
void LCD_SetOrientation(uint8_t orientation);
void LCD_WriteCommand (uint8_t index);
void LCD_WritePixel (uint16_t byte);
void LCD_ClearWindow (void);
void LCD_DrawRectangle (uint16_t X_start, uint16_t Y_start, uint16_t width, uint16_t height, uint16_t color);
void LCD_DrawCircle(uint16_t Xpos,uint16_t Ypos, uint16_t radius, uint16_t color);
uint16_t LCD_GetPixel(uint16_t i, uint16_t j);
void LCD_SetBackground(uint16_t color);
uint16_t LCD_GetBackground(void);

void LCD_DrawLineLowToHigh( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color );	//righe che partono da x e y in basso a sinistra per andare a x e y in alto a destra
void LCD_DrawLineHighToLow( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color );	//righe che partono da x e y in alto a sinistra per andare a x e y in basso destra
void LCD_DrawLine45Deg( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color );		//Righe da 45 gradi da x e y in basso a sinistra verso x e y in alto a destra
void LCD_DrawLine45DegOpposite( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color );	//Righe da 45 gradi da x e y in basso a sinistra verso x e y in basso a destra

void buildCharacter(void);						//Disegna il personaggio
void buildCharacterOption1(void);			//Disegna i soli componenti che si muovono in una posizione
void buildCharacterOption2(void);			//Disegna i soli componenti che si muovono nell'altra posizione (diversa dalla funzione precendente)
void mealSnackSpace(void);						//DIsegna lo spazio riservato a meal e snack e inserisce il nome delle due opzioni
void batteryDraw(void);								//Disegna le batterie e le riempie con le tacche (4 max)
void ageSet(void);										//Prepare la zona dedicata all'età che si aggiorna poi con il timer 1 (stesso timer usato per l'animazione)

uint16_t getHap(void);			//mi riferisce lo stato della batteria Happiness (usata nel rit)
uint16_t getSat(void);			//mi riferisce lo stato della batteria Satiety (usata nel rit)

void updateHappiness(void);		//do il meal, faccio l'animazione, compare il cibo e aumento la tacca
void createMeal(void); 				//genera il meal sullo schermo
void deleteMeal(void); 				//elimina il meal dallo schermo

void updateSatiety(void);			//do lo snack, faccio l'animazione, compare il cibo e aumento la tacca
void createSnack(void);				//genera lo snack sullo schermo
void deleteSnack(void); 			//elimina lo snack dallo schermo

void endGame(void);						//Il personaggio ha almeno una delle due batterie a 0 --> fine partita (compare l'opzione restart e spariscono meal e snack)
void deleteCharacter1(void);	//elimina componenti animazione 1
void deleteCharacter2(void);	//elimina componenti animazione 2
void deleteCharacter(void); 	//elimino il personaggio in tutto il suo splendore

void moveDownLeft1(void); 		//primo passo in diagonale in basso a sinistra
void deleteDownLeft1(void);

void moveDownLeft2(void); 		//secondo passo in diagonale in basso a sinistra (dove mangia)
void deleteDownLeft2(void);

void eat(void);						//gestisco il momento in cui viene scelto di nutrire il personaggio (resetto il timer 3 e lo attivo per far partire l'animazione)

void restart(void);				//inizio nuova "partita", generazione nuovo personaggio e reset dei valori

//extrapoint 2
void heart1(void);
void deleteHeart1(void);
void heart2(void);
void deleteHeart2(void);

void buildCharacterCuddle1(void);
void buildCharacterCuddle2(void);

void drawVolume(void);

void drawPrimaTacca(void);
void drawSecondaTacca(void);
void drawTerzaTacca(void);

void deletePrimaTacca(void);
void deleteSecondaTacca(void);
void deleteTerzaTacca(void);

#endif 

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
