/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.c
** Descriptions:		Has been tested SSD1289¡¢ILI9320¡¢R61505U¡¢SSD1298¡¢ST7781¡¢SPFD5408B¡¢ILI9325¡¢ILI9328¡¢
**						HX8346A¡¢HX8347A
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2012-3-10
** Version:					1.3
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			Paolo Bernardi
** Modified date:		03/01/2020
** Version:					2.0
** Descriptions:		simple arrangement for screen usage
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GLCD.h" 
#include "AsciiLib.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t LCD_Code;

/* Private define ------------------------------------------------------------*/
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ILI9328    2  /* 0x9328 */
#define  ILI9331    3  /* 0x9331 */
#define  SSD1298    4  /* 0x8999 */
#define  SSD1289    5  /* 0x8989 */
#define  ST7781     6  /* 0x7783 */
#define  LGDP4531   7  /* 0x4531 */
#define  SPFD5408B  8  /* 0x5408 */
#define  R61505U    9  /* 0x1505 0x0505 */
#define  HX8346A		10 /* 0x0046 */  
#define  HX8347D    11 /* 0x0047 */
#define  HX8347A    12 /* 0x0047 */	
#define  LGDP4535   13 /* 0x4535 */  
#define  SSD2119    14 /* 3.5 LCD 0x9919 */

//Global
int ss = 0;		//contatore secondi
int hh = 0;		//contatore ore
int mm = 0;		//contatore minuti

int nHappiness = 4;		//contatore tacche presenti nella batteria di happiness (restore con snack)
int nSatiety = 4;			//contatore tacche presenti nella batteria di satiey (restore con meal)

int eating = 0;		//0 --> is not eating ( i can choose meal or snack); 1 --> is eating (i cant choose meal or snack)

int dead = 0;

int cuddle = 0;

/*******************************************************************************
* Function Name  : Lcd_Configuration
* Description    : Configures LCD Control lines
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{
	/* Configure the LCD Control pins */
	
	/* EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , RS = P0.23 */
	/* RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7 */  
	LPC_GPIO0->FIODIR   |= 0x03f80000;
	LPC_GPIO0->FIOSET    = 0x03f80000;
}

/*******************************************************************************
* Function Name  : LCD_Send
* Description    : LCDÐ´Êý¾Ý
* Input          : - byte: byte to be sent
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_Send (uint16_t byte) 
{
	LPC_GPIO2->FIODIR |= 0xFF;          /* P2.0...P2.7 Output */
	LCD_DIR(1)		   				    				/* Interface A->B */
	LCD_EN(0)	                        	/* Enable 2A->2B */
	LPC_GPIO2->FIOPIN =  byte;          /* Write D0..D7 */
	LCD_LE(1)                         
	LCD_LE(0)														/* latch D0..D7	*/
	LPC_GPIO2->FIOPIN =  byte >> 8;     /* Write D8..D15 */
}

/*******************************************************************************
* Function Name  : wait_delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None 
*******************************************************************************/
static void wait_delay(int count)
{
	while(count--);
}

/*******************************************************************************
* Function Name  : LCD_Read
* Description    : LCD¶ÁÊý¾Ý
* Input          : - byte: byte to be read
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_Read (void) 
{
	uint16_t value;
	
	LPC_GPIO2->FIODIR &= ~(0xFF);              /* P2.0...P2.7 Input */
	LCD_DIR(0);		   				           				 /* Interface B->A */
	LCD_EN(0);	                               /* Enable 2B->2A */
	wait_delay(30);							   						 /* delay some times */
	value = LPC_GPIO2->FIOPIN0;                /* Read D8..D15 */
	LCD_EN(1);	                               /* Enable 1B->1A */
	wait_delay(30);							   						 /* delay some times */
	value = (value << 8) | LPC_GPIO2->FIOPIN0; /* Read D0..D7 */
	LCD_DIR(1);
	return  value;
}

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCDÐ´¼Ä´æÆ÷µØÖ·
* Input          : - index: ¼Ä´æÆ÷µØÖ·
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteIndex(uint16_t index)
{
	LCD_CS(0);
	LCD_RS(0);
	LCD_RD(1);
	LCD_Send( index ); 
	wait_delay(22);	
	LCD_WR(0);  
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCDÐ´¼Ä´æÆ÷Êý¾Ý
* Input          : - index: ¼Ä´æÆ÷Êý¾Ý
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteData(uint16_t data)
{				
	LCD_CS(0);
	LCD_RS(1);   
	LCD_Send( data );
	LCD_WR(0);     
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : ¶ÁÈ¡¿ØÖÆÆ÷Êý¾Ý
* Input          : None
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadData(void)
{ 
	uint16_t value;
	
	LCD_CS(0);
	LCD_RS(1);
	LCD_WR(1);
	LCD_RD(0);
	value = LCD_Read();
	
	LCD_RD(1);
	LCD_CS(1);
	
	return value;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{ 
	/* Write 16-bit Index, then Write Reg */  
	LCD_WriteIndex(LCD_Reg);         
	/* Write 16-bit Reg */
	LCD_WriteData(LCD_RegValue);  
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	uint16_t LCD_RAM;
	
	/* Write 16-bit Index (then Read Reg) */
	LCD_WriteIndex(LCD_Reg);
	/* Read 16-bit Reg */
	LCD_RAM = LCD_ReadData();      	
	return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
    #if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )
	
 	uint16_t temp = Xpos;

			 Xpos = Ypos;
			 Ypos = ( MAX_X - 1 ) - temp;  

	#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )
		
	#endif

  switch( LCD_Code )
  {
     default:		 /* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WriteReg(0x0020, Xpos );     
          LCD_WriteReg(0x0021, Ypos );     
	      break; 

     case SSD1298: 	 /* 0x8999 */
     case SSD1289:   /* 0x8989 */
	      LCD_WriteReg(0x004e, Xpos );      
          LCD_WriteReg(0x004f, Ypos );          
	      break;  

     case HX8346A: 	 /* 0x0046 */
     case HX8347A: 	 /* 0x0047 */
     case HX8347D: 	 /* 0x0047 */
	      LCD_WriteReg(0x02, Xpos>>8 );                                                  
	      LCD_WriteReg(0x03, Xpos );  

	      LCD_WriteReg(0x06, Ypos>>8 );                           
	      LCD_WriteReg(0x07, Ypos );    
	
	      break;     
     case SSD2119:	 /* 3.5 LCD 0x9919 */
	      break; 
  }
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void delay_ms(uint16_t ms)    
{ 
	uint16_t i,j; 
	for( i = 0; i < ms; i++ )
	{ 
		for( j = 0; j < 1141; j++ );
	}
} 


/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initialization(void)
{
	uint16_t DeviceCode;
	
	LCD_Configuration();
	delay_ms(100);
	DeviceCode = LCD_ReadReg(0x0000);		/* ¶ÁÈ¡ÆÁID	*/	
	
	if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )	
	{
		LCD_Code = ILI9325;
		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001);  	/* start internal osc */
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
		LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
		LCD_WriteReg(0x0004,0x0000);                                   
		LCD_WriteReg(0x0008,0x0207);	           
		LCD_WriteReg(0x0009,0x0000);         
		LCD_WriteReg(0x000a,0x0000); 	/* display setting */        
		LCD_WriteReg(0x000c,0x0001);	/* display setting */        
		LCD_WriteReg(0x000d,0x0000); 			        
		LCD_WriteReg(0x000f,0x0000);
		/* Power On sequence */
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);                                                                 
		LCD_WriteReg(0x0013,0x0000);                 
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0010,0x1590);   
		LCD_WriteReg(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0012,0x009c);                  
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0013,0x1900);   
		LCD_WriteReg(0x0029,0x0023);
		LCD_WriteReg(0x002b,0x000e);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);           
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0030,0x0007); 
		LCD_WriteReg(0x0031,0x0707);   
		LCD_WriteReg(0x0032,0x0006);
		LCD_WriteReg(0x0035,0x0704);
		LCD_WriteReg(0x0036,0x1f04); 
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);        
		LCD_WriteReg(0x0039,0x0706);     
		LCD_WriteReg(0x003c,0x0701);
		LCD_WriteReg(0x003d,0x000f);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0050,0x0000);        
		LCD_WriteReg(0x0051,0x00ef);   
		LCD_WriteReg(0x0052,0x0000);     
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0xa700);        
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006a,0x0000);
		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);
		  
		LCD_WriteReg(0x0090,0x0010);     
		LCD_WriteReg(0x0092,0x0000);  
		LCD_WriteReg(0x0093,0x0003);
		LCD_WriteReg(0x0095,0x0110);
		LCD_WriteReg(0x0097,0x0000);        
		LCD_WriteReg(0x0098,0x0000);  
		/* display on sequence */    
		LCD_WriteReg(0x0007,0x0133);
		
		LCD_WriteReg(0x0020,0x0000);  /* ÐÐÊ×Ö·0 */                                                          
		LCD_WriteReg(0x0021,0x0000);  /* ÁÐÊ×Ö·0 */     
	}

    delay_ms(50);   /* delay 50 ms */	
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : ½«ÆÁÄ»Ìî³ä³ÉÖ¸¶¨µÄÑÕÉ«£¬ÈçÇåÆÁ£¬ÔòÌî³ä 0xffff
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index;
	
	if( LCD_Code == HX8347D || LCD_Code == HX8347A )
	{
		LCD_WriteReg(0x02,0x00);                                                  
		LCD_WriteReg(0x03,0x00);  
		                
		LCD_WriteReg(0x04,0x00);                           
		LCD_WriteReg(0x05,0xEF);  
		                 
		LCD_WriteReg(0x06,0x00);                           
		LCD_WriteReg(0x07,0x00);    
		               
		LCD_WriteReg(0x08,0x01);                           
		LCD_WriteReg(0x09,0x3F);     
	}
	else
	{	
		LCD_SetCursor(0,0); 
	}	

	LCD_WriteIndex(0x0022);
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
		LCD_WriteData(Color);
	}
}

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB ¸ÄÎª BBBBBGGGGGGRRRRR ¸ñÊ½
* Input          : - color: BRG ÑÕÉ«Öµ  
* Output         : None
* Return         : RGB ÑÕÉ«Öµ
* Attention		 : ÄÚ²¿º¯Êýµ÷ÓÃ
*******************************************************************************/
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : »ñÈ¡Ö¸¶¨×ù±êµÄÑÕÉ«Öµ
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;
	
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteIndex(0x0022);  
	
	switch( LCD_Code )
	{
		case ST7781:
		case LGDP4531:
		case LGDP4535:
		case SSD1289:
		case SSD1298:
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  dummy;	      
	    case HX8347A:
	    case HX8347D:
             {
		        uint8_t red,green,blue;
				
				dummy = LCD_ReadData();   /* Empty read */

		        red = LCD_ReadData() >> 3; 
                green = LCD_ReadData() >> 2; 
                blue = LCD_ReadData() >> 3; 
                dummy = (uint16_t) ( ( red<<11 ) | ( green << 5 ) | blue ); 
		     }	
	         return  dummy;

        default:	/* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919 */
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  LCD_BGR2RGB( dummy );
	}
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : ÔÚÖ¸¶¨×ù±ê»­µã
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	if( Xpos >= MAX_X || Ypos >= MAX_Y )
	{
		return;
	}
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteReg(0x0022,point);
}

/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Bresenham's line algorithm
* Input          : - x1: AµãÐÐ×ù±ê
*                  - y1: AµãÁÐ×ù±ê 
*				   - x2: BµãÐÐ×ù±ê
*				   - y2: BµãÁÐ×ù±ê 
*				   - color: ÏßÑÕÉ«
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/	 
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color )
{
    short dx,dy;      /* ¶¨ÒåX YÖáÉÏÔö¼ÓµÄ±äÁ¿Öµ */
    short temp;       /* Æðµã ÖÕµã´óÐ¡±È½Ï ½»»»Êý¾ÝÊ±µÄÖÐ¼ä±äÁ¿ */

    if( x0 > x1 )     /* XÖáÉÏÆðµã´óÓÚÖÕµã ½»»»Êý¾Ý */
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;   
    }
    if( y0 > y1 )     /* YÖáÉÏÆðµã´óÓÚÖÕµã ½»»»Êý¾Ý */
    {
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;       /* XÖá·½ÏòÉÏµÄÔöÁ¿ */
	dy = y1-y0;       /* YÖá·½ÏòÉÏµÄÔöÁ¿ */

    if( dx == 0 )     /* XÖáÉÏÃ»ÓÐÔöÁ¿ »­´¹Ö±Ïß */ 
    {
        do
        { 
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ Ãè´¹Ö±Ïß */
            y0++;
        }
        while( y1 >= y0 ); 
		return; 
    }
    if( dy == 0 )     /* YÖáÉÏÃ»ÓÐÔöÁ¿ »­Ë®Æ½Ö±Ïß */ 
    {
        do
        {
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ ÃèË®Æ½Ïß */
            x0++;
        }
        while( x1 >= x0 ); 
		return;
    }
	/* ²¼À¼É­ººÄ·(Bresenham)Ëã·¨»­Ïß */
    if( dx > dy )                         /* ¿¿½üXÖá */
    {
	    temp = 2 * dy - dx;               /* ¼ÆËãÏÂ¸öµãµÄÎ»ÖÃ */         
        while( x0 != x1 )
        {
	        LCD_SetPoint(x0,y0,color);    /* »­Æðµã */ 
	        x0++;                         /* XÖáÉÏ¼Ó1 */
	        if( temp > 0 )                /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */
	        {
	            y0++;                     /* ÎªÓÒÉÏÏàÁÚµã£¬¼´£¨x0+1,y0+1£© */ 
	            temp += 2 * dy - 2 * dx; 
	 	    }
            else         
            {
			    temp += 2 * dy;           /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */  
			}       
        }
        LCD_SetPoint(x0,y0,color);
    }  
    else
    {
	    temp = 2 * dx - dy;                      /* ¿¿½üYÖá */       
        while( y0 != y1 )
        {
	 	    LCD_SetPoint(x0,y0,color);     
            y0++;                 
            if( temp > 0 )           
            {
                x0++;               
                temp+=2*dy-2*dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        } 
        LCD_SetPoint(x0,y0,color);
	}
} 

/******************************************************************************
* Function Name  : PutChar
* Description    : ½«LcdÆÁÉÏÈÎÒâÎ»ÖÃÏÔÊ¾Ò»¸ö×Ö·û
* Input          : - Xpos: Ë®Æ½×ø±ê 
*                  - Ypos: ´¹Ö±×ø±ê  
*				   - ASCI: ÏÔÊ¾µÄ×Ö·û
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor )
{
	uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* È¡×ÖÄ£Êý¾Ý */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( ((tmp_char >> (7 - j)) & 0x01) == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* ×Ö·ûÑÕÉ« */
            }
            else
            {
                LCD_SetPoint( Xpos + j, Ypos + i, bkColor );  /* ±³¾°ÑÕÉ« */
            }
        }
    }
}

/******************************************************************************
* Function Name  : GUI_Text
* Description    : ÔÚÖ¸¶¨×ù±êÏÔÊ¾×Ö·û´®
* Input          : - Xpos: ÐÐ×ù±ê
*                  - Ypos: ÁÐ×ù±ê 
*				   - str: ×Ö·û´®
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;  
        PutChar( Xpos, Ypos, TempChar, Color, bkColor );    
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        } 
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }   
        else
        {
            Xpos = 0;
            Ypos = 0;
        }    
    }
    while ( *str != 0 );
}

void drawShape(uint16_t lenght, uint16_t x0, uint16_t y0, uint16_t color) {
	int i;
	int xDX = x0;
	int	xSX = x0 + lenght;
	int y = y0;
	
	//orizzonale
	for(i=x0; i<x0+lenght; i=i+2) {
		LCD_SetPoint(i, y0, color);
	}
	
	//DIag
	while(xDX != xSX) {
		LCD_SetPoint(xDX, y, color);
		LCD_SetPoint(xSX, y, color);
		
		xDX++;
		xSX--;
		y = y-2;
	}
	LCD_SetPoint(xDX, y, color);	//Punto finale
	
}

void wait()
{
	__ASM("nop");
}

void LCD_WriteCommand (uint8_t index)
{
	LCD_RS(0)
  LCD_RD(1)
  LCD_Send(index);
  LCD_WR(0)
  wait();
  LCD_WR(1)
}

void LCD_WritePixel (uint16_t pixel)
{
	LCD_Send(pixel);
  LCD_WR(0)
  wait();
  LCD_WR(1)
}

void LCD_ClearWindow (void)
{
	/* Window Address Area */
	LCD_WriteReg(0x50,0);       									/* Set X Start 	*/
	LCD_WriteReg(0x51,239);	    									/* Set X End 		*/
	LCD_WriteReg(0x52,0);	   									  	/* Set Y Start 	*/
	LCD_WriteReg(0x53,319);										    /* Set Y End 		*/
}

void LCD_SetWindow (uint16_t X_start, uint16_t Y_start, uint16_t width, uint16_t height)
{
	/* Window Address Area	*/
	LCD_WriteReg(0x50,X_start);       						/* Set X Start	*/
	LCD_WriteReg(0x51,X_start + width - 1);				/* Set X End 		*/
	LCD_WriteReg(0x52,Y_start);	      						/* Set Y Start 	*/
	LCD_WriteReg(0x53,Y_start + height -1);				/* Set Y End 		*/
	/* Set pos */
	LCD_WriteReg(0x20, X_start);
	LCD_WriteReg(0x21, Y_start);
}

void LCD_SetOrientation(uint8_t orientation)
{
	switch (orientation)
  {
  	case 'V':
			LCD_WriteReg(0x03,0x1030);
  		break;
  	case 'H':
			LCD_WriteReg(0x03,0x1018);
  		break;
  	default:
  		break;
  }
	return;
}

void pixel(uint16_t xc,uint16_t yc,uint16_t x,uint16_t y, uint16_t color)
{
	LCD_SetPoint(xc+x,yc+y,color);
	LCD_SetPoint(xc+x,yc-y,color);
	LCD_SetPoint(xc-x,yc+y,color);
	LCD_SetPoint(xc-x,yc-y,color);
	LCD_SetPoint(xc+y,yc+x,color);
	LCD_SetPoint(xc+y,yc-x,color);
	LCD_SetPoint(xc-y,yc+x,color);
	LCD_SetPoint(xc-y,yc-x,color);
}

void LCD_DrawCircle(uint16_t Xpos,uint16_t Ypos, uint16_t radius, uint16_t color)
{
	int x=0;
	int y=radius;
	int p=1-radius;
	pixel(Xpos,Ypos,x,y, color);
	
	while(x<y)
	{
		if(p<0)
		{
			x++;
			p=p+2*x+1;
		}
		else
		{
			x++;
			y--;
			p=p+2*(x-y)+1;
		}
		pixel(Xpos,Ypos,x,y, color);
	}
}

void LCD_DrawRectangle (uint16_t X_start, uint16_t Y_start, uint16_t width, uint16_t height, uint16_t color)
{
	uint16_t i, j;
	
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			LCD_SetPoint(X_start + i, Y_start + j, color);
		}
	}
	
}

void LCD_Darken(uint16_t X_start, uint16_t Y_start, uint16_t width, uint16_t height, uint16_t darken_factor)
{
	uint16_t i, j, pixel;
	uint8_t r, g, b;
		
	for (i = X_start; i < X_start + width; i++) {
		for (j = Y_start; j < Y_start + height; j++) {
			LCD_SetCursor(i, j);
			LCD_WriteIndex(0x22);
			pixel = LCD_ReadData();
			r = (pixel & 0xF800) >> 10;
			//r = r >= darken_factor ? r - darken_factor : 0;
			g = (pixel & 0x07E0) >> 5;
			//g = g >= darken_factor ? g - darken_factor : 0;
			b = (pixel & 0x001F);
			//b = b >= darken_factor ? b - darken_factor : 0;
			pixel = RGB565CONVERT(r, g, b);
			LCD_SetPoint(i, j, pixel);
		}
	}
}

uint16_t LCD_GetPixel(uint16_t i, uint16_t j)
{
	LCD_SetCursor(i, j);
	LCD_WriteIndex(0x22);
	return LCD_ReadData();
}

void LCD_SetBackground(uint16_t color)
{
	back_color = color;
	LCD_Clear(back_color);
}

uint16_t LCD_GetBackground(void)
{
	return back_color;
}

void LCD_DrawLineLowToHigh( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color ) {
	int x = x0;
	int y = y0;
	
	while((x <= x1) && (y >= y1)) {
		LCD_SetPoint(x, y, color);
		//x++;
		x = x+2;
		y--;
	}
}

void LCD_DrawLineHighToLow( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color ) {
	int x = x0;
	int y = y0;
	
	while((x <= x1) && (y <= y1)) {
		LCD_SetPoint(x, y, color);
		//x++;
		x = x+2;
		y++;;
	}
} 

void LCD_DrawLine45Deg( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color ) {
	int x = x0;
	int y = y0;
	
	while((x <= x1) && (y >= y1)) {
		LCD_SetPoint(x, y, color);
		x++;
		y--;
	}
}

void LCD_DrawLine45DegOpposite( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color ) {
	int x = x0;
	int y = y0;
	
	while((x <= x1) && (y <= y1)) {
		LCD_SetPoint(x, y, color);
		x++;
		y++;
	}
}

//Build character option 1
void buildCharacter(void) {
	int i;
	
	drawShape(100, 70, 200, Black);									//draw triangle (body)
	
	LCD_DrawCircle(105, 150, 7, Black);							//Left eye
	LCD_DrawCircle(135, 150, 7, Black);							//right eye
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 151, i, Black);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(133, 151, i, Black);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (98, 139, 14, 2, Black);			//left eyebrows
	LCD_DrawRectangle (128, 139, 14, 2, Black);			//right eyebrows
	
	LCD_DrawLine(100, 200, 100, 220 , Black);				//left leg
	LCD_DrawLine( 140, 200, 140, 220, Black);				//right leg
	
	LCD_DrawRectangle (85, 220, 16, 8, Black);			//left foot
	
	LCD_DrawRectangle (125, 220, 16, 8, Black);			//right foot
	
	LCD_DrawLineLowToHigh(55, 185, 85, 170 , Black);		//left arm
	
	LCD_DrawLineLowToHigh(155, 170, 185, 155 , Black);	//right arm
	
	LCD_DrawRectangle (85, 115, 70, 10, Black);		//hat base
	LCD_DrawRectangle (105, 95, 30, 20, Black);		//hat top
	
	LCD_DrawLine45Deg(110, 170, 120, 160, Black); 	//nose	
	LCD_DrawLine( 110, 170, 120, 170, Black);				//nose
	
	LCD_DrawLine( 100, 185, 140, 185, Black);				//mouth
	
}

void buildCharacterOption1() {
	int i;
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(107, 151, i, White);						//delete left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 151, i, Black);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(137, 151, i, White);						//delete right eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(133, 151, i, Black);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (100, 220, 16, 8, White);			//delete left foot
	LCD_DrawRectangle (85, 220, 16, 8, Black);			//left foot
	
	LCD_DrawRectangle (140, 220, 16, 8, White);			//delete right foot
	LCD_DrawRectangle (125, 220, 16, 8, Black);			//right foot
	
	LCD_DrawLineHighToLow(55, 155, 85, 170, White);			//delete left arm
	LCD_DrawLineLowToHigh(55, 185, 85, 170 , Black);		//left arm
	
	LCD_DrawLineHighToLow(155, 170, 185, 185, White);		//delete right arm
	LCD_DrawLineLowToHigh(155, 170, 185, 155 , Black);	//right arm
	
	LCD_DrawLine45DegOpposite(120, 160, 130, 170, White);		//delete nose
	LCD_DrawLine45Deg(110, 170, 120, 160, Black); 	//nose
	
	LCD_DrawLine( 120, 170, 130, 170, White);		//delete nose
	LCD_DrawLine( 110, 170, 120, 170, Black);				//nose
}

//Build character option 2
void buildCharacterOption2(void) {
	int i;
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 151, i, White);						//delete left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(107, 151, i, Black);						//fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(133, 151, i, White);						//delete right eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(137, 151, i, Black);						//fill right eye
	}
	
	LCD_DrawRectangle (85, 220, 16, 8, White);			//left foot, DELETE old one
	LCD_DrawRectangle (100, 220, 16, 8, Black);			//left foot, CREATE new one
	
	LCD_DrawRectangle (125, 220, 16, 8, White);			//right foot, DELETE old one
	LCD_DrawRectangle (140, 220, 16, 8, Black);			//right foot, CREATE new one
	
	LCD_DrawLineLowToHigh(55, 185, 85, 170, White);		//left arm, DELETE new one
	LCD_DrawLineHighToLow(55, 155, 85, 170, Black);		//left arm, CREATE new one
	
	LCD_DrawLineLowToHigh(155, 170, 185, 155, White);	//right arm, DELETE new one
	LCD_DrawLineHighToLow(155, 170, 185, 185, Black);	//right arm, Create new one
	
	LCD_DrawLine45Deg(110, 170, 120, 160, White); 	//nose
	LCD_DrawLine45DegOpposite(120, 160, 130, 170, Black); 	//nose
	
	LCD_DrawLine( 110, 170, 120, 170, White);				//nose
	LCD_DrawLine( 120, 170, 130, 170, Black);				//nose
	
}

void mealSnackSpace(void) {
	LCD_DrawLine(0, 280, 240, 280 , Black);			//top line
	LCD_DrawLine(120, 280, 120, 320 , Black);			//middle line
	
	GUI_Text(40, 295, "Meal", Black, White);
	GUI_Text(160, 295, "Snack", Black, White);
}

void batteryDraw(void) {
	//first battery --> happiness
	LCD_DrawLine(43, 60, 76, 60, Black);					//bottom
	LCD_DrawLine(43, 45, 76, 45, Black);					//top
	LCD_DrawLine(43, 60, 43, 45, Black);					//left
	LCD_DrawLine(76, 60, 76, 45, Black);					//right
	LCD_DrawRectangle (77, 50, 2, 5, Black);		//spriulino
	
	GUI_Text(25, 25, "Happiness", Black, White);
	LCD_DrawRectangle (45, 47, 6, 12, Black);		//battery 1
	LCD_DrawRectangle (53, 47, 6, 12, Black);		//battery 2
	LCD_DrawRectangle (61, 47, 6, 12, Black);		//battery 3
	LCD_DrawRectangle (69, 47, 6, 12, Black);		//battery 4
	
	//second battery --> satiety
	LCD_DrawLine(163, 60, 196, 60, Black);					//bottom
	LCD_DrawLine(163, 45, 196, 45, Black);					//top
	LCD_DrawLine(163, 60, 163, 45, Black);					//left
	LCD_DrawLine(196, 60, 196, 45, Black);					//right
	LCD_DrawRectangle (198, 50, 2, 5, Black);		//spriulino
	
	GUI_Text(155, 25, "Satiety", Black, White);
	LCD_DrawRectangle (165, 47, 6, 12, Black);		//battery 1
	LCD_DrawRectangle (173, 47, 6, 12, Black);		//battery 2
	LCD_DrawRectangle (181, 47, 6, 12, Black);		//battery 3
	LCD_DrawRectangle (189, 47, 6, 12, Black);		//battery 4
}

void ageSet(void) {
	GUI_Text(80, 5, "Age:", Black, White);			//age
}

uint16_t getHap(void) {
	return nHappiness;
}

uint16_t getSat(void) {
	return nSatiety;
}

void updateHappiness(void) {
	nHappiness = nHappiness+1;
	switch(nHappiness) {
			case 2:
				LCD_DrawRectangle (53, 47, 6, 12, Black);	//Happiness 2
				break;
			case 3:
				LCD_DrawRectangle (61, 47, 6, 12, Black);		//Happiness 3
				break;
			case 4:
				LCD_DrawRectangle (69, 47, 6, 12, Black);		//Happiness 4
				break;
			default:
				break;
	}
}

void createMeal(void) {
	//cerchio con m dentro
	LCD_DrawCircle(35, 269, 10, Black);
	GUI_Text(31, 262, "M", Red, White);
	enable_timer(3);
}

void deleteMeal(void) {
	//cerchio con m dentro
	LCD_DrawCircle(35, 269, 10, White);
	GUI_Text(31, 262, "M", White, White);
}

void updateSatiety(void) {
	nSatiety = nSatiety + 1;
	switch(nSatiety) {
			case 2:
				LCD_DrawRectangle (173, 47, 6, 12, Black);		//Satiety 2
				break;
			case 3:
				LCD_DrawRectangle (181, 47, 6, 12, Black);		//Satiety 3
				break;
			case 4:
				LCD_DrawRectangle (189, 47, 6, 12, Black);		//Satiety 4
				break;
			default:
				break;
		}
}

void createSnack(void) {
	//rettangolo con s dentro
	LCD_DrawLine(20, 279, 45, 279, Black);					//bottom
	LCD_DrawLine(20, 262, 45, 262, Black);					//top
	LCD_DrawLine(20, 279, 20, 262, Black);					//left
	LCD_DrawLine(45, 279, 45, 262, Black);					//right
	GUI_Text(30, 263, "S", Red, White);
}

void deleteSnack(void) {
	//rettangolo con s dentro
	LCD_DrawLine(20, 279, 45, 279, White);					//bottom
	LCD_DrawLine(20, 262, 45, 262, White);					//top
	LCD_DrawLine(20, 279, 20, 262, White);					//left
	LCD_DrawLine(45, 279, 45, 262, White);					//right
	GUI_Text(30, 256, "S", White, White);
}

void restart(void) {
	dead = 0;
	
	//Riempio le batterie
	LCD_DrawRectangle (45, 47, 6, 12, Black);		//Happiness 1
	LCD_DrawRectangle (53, 47, 6, 12, Black);		//Happiness 2
	LCD_DrawRectangle (61, 47, 6, 12, Black);		//Happiness 3
	LCD_DrawRectangle (69, 47, 6, 12, Black);		//Happiness 4
	
	LCD_DrawRectangle (165, 47, 6, 12, Black);		//Satiety 1
	LCD_DrawRectangle (173, 47, 6, 12, Black);		//Satiety 2
	LCD_DrawRectangle (181, 47, 6, 12, Black);		//Satiety 3
	LCD_DrawRectangle (189, 47, 6, 12, Black);		//Satiety 4
	
	GUI_Text(55, 150, "Your Tamagotchi", White, White);
	GUI_Text(90, 170, "IS DEAD", White, White);
	
	//sistemo i timer
	reset_timer(1);
	reset_timer(2);
	enable_timer(1);
	enable_timer(2);
	reset_timer(0);
	
	//preparo la personaggio e valori
	buildCharacter();
	LCD_DrawRectangle (0, 281, 240, 39, White);
	mealSnackSpace();
	nHappiness = 4;
	nSatiety = 4;
	hh = 0;
	mm = 0;
	ss = 0;
}

void endGame(void) {
	deleteHeart2();
	deleteHeart1();
	disable_timer(1);
	LCD_DrawRectangle (0, 281, 240, 39, White);
	GUI_Text(100, 295, "Reset", Black, White);
	enable_timer(3);
}

void deleteCharacter1(void) {
	int i;

	LCD_DrawCircle(105, 150, 7, White);							//Left eye
	LCD_DrawCircle(135, 150, 7, White);							//right eye
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 151, i, White);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(133, 151, i, White);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (85, 220, 16, 8, White);			//left foot
	
	LCD_DrawRectangle (125, 220, 16, 8, White);			//right foot
	
	LCD_DrawLine45Deg(110, 170, 120, 160, White); 	//nose
	
	LCD_DrawLine( 110, 170, 120, 170, White);				//nose
	
	deleteHeart2();
	deleteHeart1();
}

void deleteCharacter2(void) {
	int i;
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(107, 151, i, White);						//fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(137, 151, i, White);						//fill right eye
	}
	
	LCD_DrawRectangle (100, 220, 16, 8, White);			//left foot, CREATE new one
	
	LCD_DrawRectangle (140, 220, 16, 8, White);			//right foot, CREATE new one
	
	LCD_DrawLineHighToLow(55, 155, 85, 170, White);		//left arm, CREATE new one
	
	LCD_DrawLineHighToLow(155, 170, 185, 185, White);	//right arm, Create new one
	
	LCD_DrawLine45DegOpposite(120, 160, 130, 170, White); 	//nose
	
	LCD_DrawLine( 120, 170, 130, 170, White);				//nose
	
	deleteHeart2();
	deleteHeart1();
}

void deleteCharacter(void) {	
	drawShape(100, 70, 200, White);									//draw triangle (body)
	
	LCD_DrawRectangle (98, 139, 14, 2, White);			//left eyebrows
	LCD_DrawRectangle (128, 139, 14, 2, White);			//right eyebrows
	
	LCD_DrawLine(100, 200, 100, 220 , White);				//left leg
	LCD_DrawLine( 140, 200, 140, 220, White);				//right leg
	
	LCD_DrawLineLowToHigh(55, 185, 85, 170 , White);		//left arm
	
	LCD_DrawLineLowToHigh(155, 170, 185, 155 , White);	//right arm
	
	LCD_DrawRectangle (85, 115, 70, 10, White);		//hat base
	LCD_DrawRectangle (105, 95, 30, 20, White);		//hat top
	
	LCD_DrawLine( 100, 185, 140, 185, White);				//mouth	
	
	deleteHeart2();
	deleteHeart1();
}

void eat(void) {
	reset_timer(3);
	enable_timer(3);
}

void moveDownLeft1(void) {
	int i;
	
	drawShape(100, 40, 215, Black);									//draw triangle (body) lenght x0 y0
	
	LCD_DrawCircle(75, 165, 7, Black);							//Left eye			
	LCD_DrawCircle(105, 165, 7, Black);							//right eye
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(73, 166, i, Black);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 166, i, Black);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (68, 154, 14, 2, Black);			//left eyebrows
	LCD_DrawRectangle (98, 154, 14, 2, Black);			//right eyebrows
	
	LCD_DrawLine(70, 215, 70, 235 , Black);				//left leg
	LCD_DrawLine( 110, 215, 110, 235, Black);				//right leg
	
	LCD_DrawRectangle (55, 235, 16, 8, Black);			//left foot
	
	LCD_DrawRectangle (95, 235, 16, 8, Black);			//right foot
	
	LCD_DrawLineLowToHigh(25, 200, 55, 185 , Black);		//left arm
	
	LCD_DrawLineLowToHigh(125, 185, 155, 170 , Black);	//right arm
	
	LCD_DrawRectangle (55, 130, 70, 10, Black);		//hat base
	LCD_DrawRectangle (75, 110, 30, 20, Black);		//hat top
	
	LCD_DrawLine45Deg(80, 185, 90, 175, Black); 	//nose	
	LCD_DrawLine( 80, 185, 90, 185, Black);				//nose
	
	LCD_DrawLine( 70, 200, 110, 200, Black);				//mouth
}

void deleteDownLeft1(void) {
	int i;
	
	drawShape(100, 40, 215, White);									//draw triangle (body) lenght x0 y0
	
	LCD_DrawCircle(75, 165, 7, White);							//Left eye			
	LCD_DrawCircle(105, 165, 7, White);							//right eye
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(73, 166, i, White);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 166, i, White);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (68, 154, 14, 2, White);			//left eyebrows
	LCD_DrawRectangle (98, 154, 14, 2, White);			//right eyebrows
	
	LCD_DrawLine(70, 215, 70, 235 , White);				//left leg
	LCD_DrawLine( 110, 215, 110, 235, White);				//right leg
	
	LCD_DrawRectangle (55, 235, 16, 8, White);			//left foot
	
	LCD_DrawRectangle (95, 235, 16, 8, White);			//right foot
	
	LCD_DrawLineLowToHigh(25, 200, 55, 185 , White);		//left arm
	
	LCD_DrawLineLowToHigh(125, 185, 155, 170 , White);	//right arm
	
	LCD_DrawRectangle (55, 130, 70, 10, White);		//hat base
	LCD_DrawRectangle (75, 110, 30, 20, White);		//hat top
	
	LCD_DrawLine45Deg(80, 185, 90, 175, White); 	//nose	
	LCD_DrawLine( 80, 185, 90, 185, White);				//nose
	
	LCD_DrawLine( 70, 200, 110, 200, White);				//mouth	
}

void moveDownLeft2(void) {
	int i;
	
	drawShape(100, 10, 230, Black);									//draw triangle (body) lenght x0 y0
	
	LCD_DrawCircle(45, 180, 7, Black);							//Left eye			
	LCD_DrawCircle(75, 180, 7, Black);							//right eye
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(43, 181, i, Black);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(73, 181, i, Black);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (38, 169, 14, 2, Black);			//left eyebrows
	LCD_DrawRectangle (68, 169, 14, 2, Black);			//right eyebrows
	
	LCD_DrawLine(40, 230, 40, 250 , Black);				//left leg
	LCD_DrawLine(80, 230, 80, 250, Black);				//right leg
	
	LCD_DrawRectangle (25, 250, 16, 8, Black);			//left foot
	
	LCD_DrawRectangle (65, 250, 16, 8, Black);			//right foot
	
	LCD_DrawLineLowToHigh(0, 215, 25, 200 , Black);		//left arm
	
	LCD_DrawLineLowToHigh(95, 200, 125, 185, Black);	//right arm         
	
	LCD_DrawRectangle (25, 145, 70, 10, Black);		//hat base
	LCD_DrawRectangle (45, 125, 30, 20, Black);		//hat top
	
	LCD_DrawLine45Deg(50, 200, 60, 190, Black); 	//nose	
	LCD_DrawLine(50, 200, 60, 200, Black);				//nose
	
	LCD_DrawLine(40, 215, 80, 215, Black);				//mouth
}

void deleteDownLeft2(void) {
	int i;
	
	drawShape(100, 10, 230, White);									//draw triangle (body) lenght x0 y0
	
	LCD_DrawCircle(45, 180, 7, White);							//Left eye			
	LCD_DrawCircle(75, 180, 7, White);							//right eye
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(43, 181, i, White);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(73, 181, i, White);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (38, 169, 14, 2, White);			//left eyebrows
	LCD_DrawRectangle (68, 169, 14, 2, White);			//right eyebrows
	
	LCD_DrawLine(40, 230, 40, 250 , White);				//left leg
	LCD_DrawLine(80, 230, 80, 250, White);				//right leg
	
	LCD_DrawRectangle (25, 250, 16, 8, White);			//left foot
	
	LCD_DrawRectangle (65, 250, 16, 8, White);			//right foot
	
	LCD_DrawLineLowToHigh(0, 215, 25, 200 , White);		//left arm
	
	LCD_DrawLineLowToHigh(95, 200, 125, 185, White);	//right arm         
	
	LCD_DrawRectangle (25, 145, 70, 10, White);		//hat base
	LCD_DrawRectangle (45, 125, 30, 20, White);		//hat top
	
	LCD_DrawLine45Deg(50, 200, 60, 190, White); 	//nose	
	LCD_DrawLine(50, 200, 60, 200, White);				//nose
	
	LCD_DrawLine(40, 215, 80, 215, White);				//mouth
}

//extrapoint 2

void heart1(void) {
	//165 x      140 y (centro)
	//contorni
	LCD_SetPoint(165, 140, Black); 	//centro del cuore in alto
	
	LCD_SetPoint(164, 139, Black);	//sx
	LCD_SetPoint(166, 139, Black);	//dx
	
	LCD_DrawLine(162, 138, 163, 138, Black); //top left
	LCD_DrawLine(167, 138, 168, 138, Black);	//top right
	
	LCD_SetPoint(161, 139, Black);	//sx
	LCD_SetPoint(169, 139, Black);	//dx
	
	LCD_DrawLine(160, 140, 160, 141, Black); 	//vertical row
	LCD_DrawLine(170, 140, 170, 141, Black);	//vertival row
	
	LCD_SetPoint(161, 142, Black);	//sx
	LCD_SetPoint(169, 142, Black);	//dx
	
	LCD_SetPoint(162, 143, Black);	//sx
	LCD_SetPoint(168, 143, Black);	//dx
	
	LCD_SetPoint(163, 144, Black);	//sx
	LCD_SetPoint(167, 144, Black);	//dx
	
	LCD_SetPoint(164, 145, Black);	//sx
	LCD_SetPoint(166, 145, Black);	//dx
	
	LCD_SetPoint(165, 146, Black);	//centro in fondo
	
	//riempimento (top -> down)
	LCD_DrawLine(162, 139, 163, 139, Red); //top left
	LCD_DrawLine(167, 139, 168, 139, Red);	//top right
	
	LCD_DrawLine(161, 140, 164, 140, Red); //left
	LCD_DrawLine(166, 140, 169, 140, Red);	//right
	
	LCD_DrawLine(161, 141, 169, 141, Red);
	
	LCD_DrawLine(162, 142, 168, 142, Red);
	
	LCD_DrawLine(163, 143, 167, 143, Red);
	
	LCD_DrawLine(164, 144, 166, 144, Red);
	
	LCD_SetPoint(165, 145, Red);
}

void deleteHeart1(void) {
	//contorni
	LCD_SetPoint(165, 140, White); 	//centro del cuore in alto
	
	LCD_SetPoint(164, 139, White);	//sx
	LCD_SetPoint(166, 139, White);	//dx
	
	LCD_DrawLine(162, 138, 163, 138, White); //top left
	LCD_DrawLine(167, 138, 168, 138, White);	//top right
	
	LCD_SetPoint(161, 139, White);	//sx
	LCD_SetPoint(169, 139, White);	//dx
	
	LCD_DrawLine(160, 140, 160, 141, White); 	//vertical row
	LCD_DrawLine(170, 140, 170, 141, White);	//vertival row
	
	LCD_SetPoint(161, 142, White);	//sx
	LCD_SetPoint(169, 142, White);	//dx
	
	LCD_SetPoint(162, 143, White);	//sx
	LCD_SetPoint(168, 143, White);	//dx
	
	LCD_SetPoint(163, 144, White);	//sx
	LCD_SetPoint(167, 144, White);	//dx
	
	LCD_SetPoint(164, 145, White);	//sx
	LCD_SetPoint(166, 145, White);	//dx
	
	LCD_SetPoint(165, 146, White);	//centro in fondo
	
	//riempimento (top -> down)
	LCD_DrawLine(162, 139, 163, 139, White); //top left
	LCD_DrawLine(167, 139, 168, 139, White);	//top right
	
	LCD_DrawLine(161, 140, 164, 140, White); //left
	LCD_DrawLine(166, 140, 169, 140, White);	//right
	
	LCD_DrawLine(161, 141, 169, 141, White);
	
	LCD_DrawLine(162, 142, 168, 142, White);
	
	LCD_DrawLine(163, 143, 167, 143, White);
	
	LCD_DrawLine(164, 144, 166, 144, White);
	
	LCD_SetPoint(165, 145, White);
}

void heart2(void) {
	//contorni
	LCD_SetPoint(165, 120, Black); 	//centro del cuore in alto
	
	LCD_SetPoint(164, 119, Black);	//sx
	LCD_SetPoint(166, 119, Black);	//dx
	
	LCD_DrawLine(162, 118, 163, 118, Black); //top left
	LCD_DrawLine(167, 118, 168, 118, Black);	//top right
	
	LCD_SetPoint(161, 119, Black);	//sx
	LCD_SetPoint(169, 119, Black);	//dx
	
	LCD_DrawLine(160, 120, 160, 121, Black); 	//vertical row
	LCD_DrawLine(170, 120, 170, 121, Black);	//vertival row
	
	LCD_SetPoint(161, 122, Black);	//sx
	LCD_SetPoint(169, 122, Black);	//dx
	
	LCD_SetPoint(162, 123, Black);	//sx
	LCD_SetPoint(168, 123, Black);	//dx
	
	LCD_SetPoint(163, 124, Black);	//sx
	LCD_SetPoint(167, 124, Black);	//dx
	
	LCD_SetPoint(164, 125, Black);	//sx
	LCD_SetPoint(166, 125, Black);	//dx
	
	LCD_SetPoint(165, 126, Black);	//centro in fondo
	
	//riempimento (top -> down)
	LCD_DrawLine(162, 119, 163, 119, Red); //top left
	LCD_DrawLine(167, 119, 168, 119, Red);	//top right
	
	LCD_DrawLine(161, 120, 164, 120, Red); //left
	LCD_DrawLine(166, 120, 169, 120, Red);	//right
	
	LCD_DrawLine(161, 121, 169, 121, Red);
	
	LCD_DrawLine(162, 122, 168, 122, Red);
	
	LCD_DrawLine(163, 123, 167, 123, Red);
	
	LCD_DrawLine(164, 124, 166, 124, Red);
	
	LCD_SetPoint(165, 125, Red);
}

void deleteHeart2(void) {
	//contorni
	LCD_SetPoint(165, 120, White); 	//centro del cuore in alto
	
	LCD_SetPoint(164, 119, White);	//sx
	LCD_SetPoint(166, 119, White);	//dx
	
	LCD_DrawLine(162, 118, 163, 118, White); //top left
	LCD_DrawLine(167, 118, 168, 118, White);	//top right
	
	LCD_SetPoint(161, 119, White);	//sx
	LCD_SetPoint(169, 119, White);	//dx
	
	LCD_DrawLine(160, 120, 160, 121, White); 	//vertical row
	LCD_DrawLine(170, 120, 170, 121, White);	//vertival row
	
	LCD_SetPoint(161, 122, White);	//sx
	LCD_SetPoint(169, 122, White);	//dx
	
	LCD_SetPoint(162, 123, White);	//sx
	LCD_SetPoint(168, 123, White);	//dx
	
	LCD_SetPoint(163, 124, White);	//sx
	LCD_SetPoint(167, 124, White);	//dx
	
	LCD_SetPoint(164, 125, White);	//sx
	LCD_SetPoint(166, 125, White);	//dx
	
	LCD_SetPoint(165, 126, White);	//centro in fondo
	
	//riempimento (top -> down)
	LCD_DrawLine(162, 119, 163, 119, White); //top left
	LCD_DrawLine(167, 119, 168, 119, White);	//top right
	
	LCD_DrawLine(161, 120, 164, 120, White); //left
	LCD_DrawLine(166, 120, 169, 120, White);	//right
	
	LCD_DrawLine(161, 121, 169, 121, White);
	
	LCD_DrawLine(162, 122, 168, 122, White);
	
	LCD_DrawLine(163, 123, 167, 123, White);
	
	LCD_DrawLine(164, 124, 166, 124, White);
	
	LCD_SetPoint(165, 125, White);
}

void buildCharacterCuddle1() {
	int i;
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(107, 151, i, White);						//delete left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 151, i, Black);						//105, 153, i, Black	  fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(137, 151, i, White);						//delete right eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(133, 151, i, Black);						//135, 153, i, Black		fill right eye
	}
	
	LCD_DrawRectangle (100, 220, 16, 8, White);			//delete left foot
	LCD_DrawRectangle (85, 220, 16, 8, Black);			//left foot
	
	LCD_DrawRectangle (140, 220, 16, 8, White);			//delete right foot
	LCD_DrawRectangle (125, 220, 16, 8, Black);			//right foot
	
	LCD_DrawLineHighToLow(55, 155, 85, 170, White);			//delete left arm
	LCD_DrawLineLowToHigh(55, 185, 85, 170 , Black);		//left arm
	
	LCD_DrawLineHighToLow(155, 170, 185, 185, Black);		//right arm
	LCD_DrawLineLowToHigh(155, 170, 185, 155 , White);	//delete right arm
	
	LCD_DrawLine45DegOpposite(120, 160, 130, 170, White);		//delete nose
	LCD_DrawLine45Deg(110, 170, 120, 160, Black); 	//nose
	
	LCD_DrawLine( 120, 170, 130, 170, White);		//delete nose
	LCD_DrawLine( 110, 170, 120, 170, Black);				//nose
}

//Build character option 2
void buildCharacterCuddle2(void) {
	int i;
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(103, 151, i, White);						//delete left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(107, 151, i, Black);						//fill left eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(133, 151, i, White);						//delete right eye
	}
	
	for(i=0; i<5; i++) {
		LCD_DrawCircle(137, 151, i, Black);						//fill right eye
	}
	
	LCD_DrawRectangle (85, 220, 16, 8, White);			//left foot, DELETE old one
	LCD_DrawRectangle (100, 220, 16, 8, Black);			//left foot, CREATE new one
	
	LCD_DrawRectangle (125, 220, 16, 8, White);			//right foot, DELETE old one
	LCD_DrawRectangle (140, 220, 16, 8, Black);			//right foot, CREATE new one
	
	LCD_DrawLineLowToHigh(55, 185, 85, 170, White);		//left arm, DELETE new one
	LCD_DrawLineHighToLow(55, 155, 85, 170, Black);		//left arm, CREATE new one
	
	LCD_DrawLineLowToHigh(155, 170, 185, 155, Black);	//right arm, DELETE new one
	LCD_DrawLineHighToLow(155, 170, 185, 185, White);	//right arm, Create new one
	
	LCD_DrawLine45Deg(110, 170, 120, 160, White); 	//nose
	LCD_DrawLine45DegOpposite(120, 160, 130, 170, Black); 	//nose
	
	LCD_DrawLine( 110, 170, 120, 170, White);				//nose
	LCD_DrawLine( 120, 170, 130, 170, Black);				//nose
	
}

void drawVolume(void) {
	//contorni
	LCD_DrawLine( 10, 2, 10, 17, Black);		//verticale più lunga
	
	LCD_SetPoint(9, 3, Black);	//top
	LCD_SetPoint(8, 4, Black);	//top
	LCD_SetPoint(7, 5, Black);	//top
	
	LCD_SetPoint(9, 16, Black);	//down
	LCD_SetPoint(8, 15, Black);	//down
	LCD_SetPoint(7, 14, Black);	//down
	
	LCD_DrawLine(3, 6, 6, 6, Black);		//orizzontale
	LCD_DrawLine(3, 13, 6, 13, Black);	//orizzontale
	
	LCD_DrawLine(2, 6, 2, 13, Black);	//verticale sx
	
	//tacca piccola
	LCD_SetPoint(12, 7, Black);
	LCD_SetPoint(12, 12, Black);	
	
	LCD_DrawLine(13, 8, 13, 11, Black);
	
	//tacca media
	LCD_SetPoint(14, 5, Black);
	LCD_SetPoint(15, 6, Black);
	
	LCD_SetPoint(14, 14, Black);
	LCD_SetPoint(15, 13, Black);
	
	LCD_DrawLine(16, 7, 16, 12, Black);
	
	//tacca grande
	LCD_SetPoint(17, 4, Black);
	LCD_SetPoint(17, 15, Black);
	
	LCD_DrawLine(18, 5, 18, 6, Black);
	LCD_DrawLine(18, 13, 18, 14, Black);
	
	LCD_DrawLine(19, 7, 19, 12, Black);
	
	//riempimento
	LCD_DrawLine(9, 4, 9, 15, Black);
	LCD_DrawLine(8, 5, 8, 14, Black);
	
	LCD_DrawLine(7, 8, 7, 13, Black);
	LCD_SetPoint(7, 6, Black);
	
	LCD_DrawLine(6, 9, 6, 12, Black);
	LCD_SetPoint(6, 7, Black);
	
	LCD_DrawLine(5, 9, 5, 12, Black);
	LCD_SetPoint(5, 7, Black);
	
	LCD_DrawLine(4, 9, 4, 12, Black);
	LCD_SetPoint(4, 7, Black);
	
	LCD_DrawLine(3, 7, 3, 12, Black);
}

void drawPrimaTacca(void) {
	LCD_SetPoint(12, 7, Black);
	LCD_SetPoint(12, 12, Black);	
	
	LCD_DrawLine(13, 8, 13, 11, Black);
}

void deletePrimaTacca(void) {
	LCD_SetPoint(12, 7, White);
	LCD_SetPoint(12, 12, White);	
	
	LCD_DrawLine(13, 8, 13, 11, White);
}

void drawSecondaTacca(void) {
	LCD_SetPoint(14, 5, Black);
	LCD_SetPoint(15, 6, Black);
	
	LCD_SetPoint(14, 14, Black);
	LCD_SetPoint(15, 13, Black);
	
	LCD_DrawLine(16, 7, 16, 12, Black);
}

void deleteSecondaTacca(void) {
	LCD_SetPoint(14, 5, White);
	LCD_SetPoint(15, 6, White);
	
	LCD_SetPoint(14, 14, White);
	LCD_SetPoint(15, 13, White);
	
	LCD_DrawLine(16, 7, 16, 12, White);
}

void drawTerzaTacca(void) {
	LCD_SetPoint(17, 4, Black);
	LCD_SetPoint(17, 15, Black);
	
	LCD_DrawLine(18, 5, 18, 6, Black);
	LCD_DrawLine(18, 13, 18, 14, Black);
	
	LCD_DrawLine(19, 7, 19, 12, Black);
}

void deleteTerzaTacca(void) {
	LCD_SetPoint(17, 4, White);
	LCD_SetPoint(17, 15, White);
	
	LCD_DrawLine(18, 5, 18, 6, White);
	LCD_DrawLine(18, 13, 18, 14, White);
	
	LCD_DrawLine(19, 7, 19, 12, White);
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
