/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvI2C.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "LCD_Driver.h"
#include "ScanKey.h"
#include "Seven_Segment.h"

void delay_loop(void)
 {
 uint32_t i;
	for(i=0;i<8000;i++);
 
 }

/*----------------------------------------------------------------------------
  Interrupt subroutine
  ----------------------------------------------------------------------------*/
static unsigned char count=0;
static unsigned char loop=12;
static uint32_t time = 0;
void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
    unsigned char i=0;
 	TIMER0->TISR.TIF =1;
	count++;
	if(count==5)
	{
	   	DrvGPIO_ClrBit(E_GPC,loop);
	   	loop++;
	   	count=0;	  
		++time; 
	   	if(loop==17)
	   	{			
	   		for(i=12;i<16;i++)
		   	{
	   			DrvGPIO_SetBit(E_GPC,i);	   
	   		}
			loop=12;
	   }
	}
}

void Timer_initial(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
    SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=0;	// Set Prescale [0~255]
	TIMER0->TCMPR  = 1000000;		// Set TICR(TCMP) [0~16777215]
								// (1/22118400)*(0+1)*(2765)= 125.01usec or 7999.42Hz

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}		   

static int current_index = 3;
static uint8_t current_key[4] = {16, 16, 16, 16};

void next_index()
{			 
	if (current_index != 0)
	{
		--current_index;
	}
	else
	{
		current_index = 4;
	}
}
void prev_index()
{			 
	if (current_index != 4)
	{
		++current_index;
	}
	else
	{
		current_index = 0;
	}
}

uint8_t get_pressed_number()
{	 
	uint8_t pressed_button = 0;
	uint8_t pressed_number = 16;
	
	pressed_button = Scankey();	
	if (pressed_button == 7)
	{
		pressed_number = 97;
	}
	else if (pressed_button == 8)
	{
		pressed_number = 98;
	}
	else if (pressed_number == 9)
	{
		pressed_number = 99;
	}
	else if (current_index != 4)
	{
		if (time < 4)
		{
			switch (pressed_button)
			{
				case 1 :
				{
					if (current_key[current_index] == 0 || current_key[current_index] == 1)
					{
						pressed_number = 1;
					}
					else
					{
						pressed_number = 0;
						next_index();
					}
					break;
				}
				case 2 :
				{
					if (current_key[current_index] == 2)
					{
						pressed_number = 3;
					}
					else if (current_key[current_index] == 3 || current_key[current_index] == 4)
					{
						pressed_number = 4;
					}
					else
					{
						pressed_number = 2;
						next_index();
					}
					break;
				}
				case 3 :
				{
					if (current_key[current_index] == 5)
					{
						pressed_number = 6;
					}
					else if (current_key[current_index] == 6 || current_key[current_index] == 7)
					{
						pressed_number = 7;
					}
					else
					{
						pressed_number = 5;
						next_index();
					}
					break;
				}
				case 4 :
				{
					if (current_key[current_index] == 8 || current_key[current_index] == 9)
					{
						pressed_number = 9;
					}
					else
					{
						pressed_number = 8;
						next_index();
					}
					break;
				}
				case 5 :
				{
					if (current_key[current_index] == 10)
					{
						pressed_number = 11;
					}
					else if (current_key[current_index] == 11 || current_key[current_index] == 12)
					{
						pressed_number = 12;
					}
					else
					{
						pressed_number = 10;
						next_index();
					}
					break;
				}
				case 6 :
				{
					if (current_key[current_index] == 13)
					{
						pressed_number = 14;
					}
					else if (current_key[current_index] == 14 || current_key[current_index] == 15)
					{
						pressed_number = 15;
					}
					else
					{
						pressed_number = 13;
						next_index();
					}
					break;
				}
			}
		}	
		else
		{
			switch (pressed_button)
			{
				case 1 :
				{
					pressed_number = 0;
					break;
				}
				case 2 :
				{
					pressed_number = 2;
					break;
				}
				case 3 :
				{
					pressed_number = 5;
					break;
				}
				case 4 :
				{
					pressed_number = 8;
					break;
				}
				case 5 :
				{
					pressed_number = 10;
					break;
				}
				case 6 :
				{
					pressed_number = 13;
					break;
				}
			}
			if (pressed_button != 0)
			{
				next_index();
			}
		}
	}
	if (pressed_button != 0)
	{
		time = 0;
	}
	return pressed_number;
}

int main(void)
{
	uint8_t pressed_number = 0x00;
	char temp[10];

	 int i=0,j=0;
	/* Unlock the protected registers */	
	UNLOCKREG();
   	/* Enable the 12MHz oscillator oscillation */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
 
     /* Waiting for 12M Xtal stalble */
    SysTimerDelay(5000);
 
	/* HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator */
	DrvSYS_SelectHCLKSource(0);		
    /*lock the protected registers */
	LOCKREG();				

	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); /* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */

    for(i=12;i<16;i++)
	{		
		DrvGPIO_Open(E_GPC, i, E_IO_OUTPUT);
    }

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	
	/*print_lcd(0, "Welcome! Nuvoton");	  
	print_lcd(1, "This is LB test ");
	print_lcd(2, "FW name:        ");	  
	print_lcd(3, "  Smp_Drv_LB.bin");*/	  	  
	Timer_initial();

	OpenKeyPad();
	
	while(1)
	{
		pressed_number = get_pressed_number();
		if (pressed_number < 16)
		{
			current_key[current_index] = pressed_number;
		}
		for(j=0;j<4;j++)
		{
			close_seven_segment();
			show_seven_segment(j,current_key[j]);
			delay_loop();
		}
	} 		
}


