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
	for(i=0;i<6000;i++);
 
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

static int current_page = 0;
static int current_position = 3;
static uint8_t current_key[4] = {16, 16, 16, 16};
static int memory_reset_flag = 0;
static char lcd[3][16];

void read_memory()
{
	int i = 0, value = 0, start = 0, start_register;
	uint8_t key_count = 0;
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	key_count = Read_24LC64(0x00000000);
	if (current_page == 0)
	{
		sprintf(lcd[0], "%s", "Kunci tersimpan:");
		sprintf(lcd[1], "%s", " ");
		sprintf(lcd[2], "%s", " ");
		if (key_count >= 1)
		{
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000001);
			sprintf(lcd[1] + 1, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000002);
			sprintf(lcd[1] + 2, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000003);
			sprintf(lcd[1] + 3, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000004);
			sprintf(lcd[1] + 4, "%x", value);
		}
		if (key_count >= 2)
		{
			sprintf(lcd[1] + 5, "%c", ' ');
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000005);
			sprintf(lcd[1] + 6, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000006);
			sprintf(lcd[1] + 7, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000007);
			sprintf(lcd[1] + 8, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000008);
			sprintf(lcd[1] + 9, "%x", value);
		}
		if (key_count >= 3)
		{
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000009);
			sprintf(lcd[2] + 1, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x0000000A);
			sprintf(lcd[2] + 2, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x0000000B);
			sprintf(lcd[2] + 3, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x0000000C);
			sprintf(lcd[2] + 4, "%x", value);
		}
		if (key_count >= 4)
		{
			sprintf(lcd[2] + 5, "%c", ' ');
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x0000000D);
			sprintf(lcd[2] + 6, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x0000000E);
			sprintf(lcd[2] + 7, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x0000000F);
			sprintf(lcd[2] + 8, "%x", value);
			DrvGPIO_InitFunction(E_FUNC_I2C1);
			value = Read_24LC64(0x00000010);
			sprintf(lcd[2] + 9, "%x", value);
		}
	}
	else
	{
		start = 4 + 6 * (current_page - 1);
		sprintf(lcd[0], "%s", " ");
		sprintf(lcd[1], "%s", " ");
		sprintf(lcd[2], "%s", " ");
		if (start + 1 > key_count)
		{
			current_page = 0;
			read_memory();
		}
		else
		{
			start_register = start * 4 + 1;
			if (key_count - start >= 1)
			{
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register);
				sprintf(lcd[0] + 1, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 1);
				sprintf(lcd[0] + 2, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 2);
				sprintf(lcd[0] + 3, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 3);
				sprintf(lcd[0] + 4, "%x", value);
			}
			if (key_count - start >= 2)
			{
				sprintf(lcd[0] + 5, "%c", ' ');
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 4);
				sprintf(lcd[0] + 6, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 5);
				sprintf(lcd[0] + 7, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 6);
				sprintf(lcd[0] + 8, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 7);
				sprintf(lcd[0] + 9, "%x", value);
			}
			if (key_count - start >= 3)
			{
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 8);
				sprintf(lcd[1] + 1, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 9);
				sprintf(lcd[1] + 2, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 10);
				sprintf(lcd[1] + 3, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 11);
				sprintf(lcd[1] + 4, "%x", value);
			}
			if (key_count - start >= 4)
			{
				sprintf(lcd[1] + 5, "%c", ' ');
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 12);
				sprintf(lcd[1] + 6, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 13);
				sprintf(lcd[1] + 7, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 14);
				sprintf(lcd[1] + 8, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 15);
				sprintf(lcd[1] + 9, "%x", value);
			}
			if (key_count - start >= 5)
			{
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 16);
				sprintf(lcd[2] + 1, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 17);
				sprintf(lcd[2] + 2, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 18);
				sprintf(lcd[2] + 3, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 19);
				sprintf(lcd[2] + 4, "%x", value);
			}
			if (key_count - start >= 6)
			{
				sprintf(lcd[2] + 5, "%c", ' ');
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 20);
				sprintf(lcd[2] + 6, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 21);
				sprintf(lcd[2] + 7, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 22);
				sprintf(lcd[2] + 8, "%x", value);
				DrvGPIO_InitFunction(E_FUNC_I2C1);
				value = Read_24LC64(0x00000000 + start_register + 23);
				sprintf(lcd[2] + 9, "%x", value);
			}
		}
	}
}

void write_memory()
{
	int i = 0, start = 0;
	uint8_t key_count = 0, value;
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	key_count = Read_24LC64(0x00000000);
	start = key_count * 4 + 1;
	for (i = 0; i < 4; i++)
	{
		DrvGPIO_InitFunction(E_FUNC_I2C1);
		Write_24LC64(0x00000000 + start + i, current_key[3 - i]);
	}
	++key_count;
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	Write_24LC64(0x00000000, key_count);

}

void reset_memory()
{
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	Write_24LC64(0x00000000, 0);
}

void print_to_lcd()
{
	int i = 0;
	read_memory();
	clr_all_pannal();
	for (i = 0; i < 3; i++)
	{
		print_lcd(i, lcd[i]);
	}
}

void next_position()
{			 
	if (current_position != 0 && current_position != 4)
	{
		--current_position;
	}
	else
	{
		current_position = 4;
	}
}
void prev_position()
{			 
	if (current_position == 4)
	{
		current_position = 0;
	}
	else if (current_position != 3)
	{
		++current_position;
	}
	else
	{
		current_position = 3;
	}
}

void process_keypress()
{
	int i = 0;
	uint8_t pressed_button = 0;
	uint8_t pressed_number = 16;
	
	pressed_button = Scankey();	
	if (pressed_button == 7)
	{
		if (current_position == 4)
		{
			prev_position();
		}
		if (current_key[current_position] == 16)
		{
			prev_position();
		}
		current_key[current_position] = 16;
	}
	else if (pressed_button == 8)
	{
		if (current_key[0] != 16 && current_key[1] != 16 && current_key[2] != 16 && current_key[3] != 16)
		{
			write_memory();
			print_to_lcd();
		}
		else if (current_key[0] == 16 && current_key[1] == 16 && current_key[2] == 16 && current_key[3] == 16)
		{
			if (time < 3 && memory_reset_flag == 1)
			{
				reset_memory();
				print_to_lcd();
				memory_reset_flag = 0;
			}
			else
			{
				memory_reset_flag = 1;
			}
		}
	}
	else if (pressed_button == 9)
	{
		++current_page;
		print_to_lcd();
	}
	else if (current_position != 4)
	{
		if (time < 3)
		{
			switch (pressed_button)
			{
				case 1 :
				{
					if (current_key[current_position] == 0 || current_key[current_position] == 1)
					{
						pressed_number = 1;
					}
					else
					{
						if (current_key[current_position] != 16)
						{
							next_position();
						}
						pressed_number = 0;
					}
					break;
				}
				case 2 :
				{
					if (current_key[current_position] == 2)
					{
						pressed_number = 3;
					}
					else if (current_key[current_position] == 3 || current_key[current_position] == 4)
					{
						pressed_number = 4;
					}
					else
					{
						if (current_key[current_position] != 16)
						{
							next_position();
						}
						pressed_number = 2;
					}
					break;
				}
				case 3 :
				{
					if (current_key[current_position] == 5)
					{
						pressed_number = 6;
					}
					else if (current_key[current_position] == 6 || current_key[current_position] == 7)
					{
						pressed_number = 7;
					}
					else
					{					
						if (current_key[current_position] != 16)
						{
							next_position();
						}
						pressed_number = 5;
					}
					break;
				}
				case 4 :
				{
					if (current_key[current_position] == 8 || current_key[current_position] == 9)
					{
						pressed_number = 9;
					}
					else
					{					
						if (current_key[current_position] != 16)
						{
							next_position();
						}
						pressed_number = 8;
					}
					break;
				}
				case 5 :
				{
					if (current_key[current_position] == 10)
					{
						pressed_number = 11;
					}
					else if (current_key[current_position] == 11 || current_key[current_position] == 12)
					{
						pressed_number = 12;
					}
					else
					{					
						if (current_key[current_position] != 16)
						{
							next_position();
						}
						pressed_number = 10;
					}
					break;
				}
				case 6 :
				{
					if (current_key[current_position] == 13)
					{
						pressed_number = 14;
					}
					else if (current_key[current_position] == 14 || current_key[current_position] == 15)
					{
						pressed_number = 15;
					}
					else
					{					
						if (current_key[current_position] != 16)
						{
							next_position();
						}
						pressed_number = 13;
					}
					break;
				}
			}
		}	
		else
		{
			memory_reset_flag = 0;
			if (current_key[current_position] != 16)
			{
				next_position();
			}
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
		}
	}
	if (pressed_button != 0)
	{
		if (pressed_number != 16)
		{
			current_key[current_position] = pressed_number;
		}
		for(i = 0; i < 100; i++)
		{
			delay_loop();
		}
		time = 0;
	}
}

int main(void)
{
	int i = 0;

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
		  	  
	Timer_initial();
	OpenKeyPad();

	print_to_lcd();
	
	while(1)
	{
		process_keypress();
		for (i = 0; i < 4; i++)
		{
			close_seven_segment();
			show_seven_segment(i, current_key[i]);
			delay_loop();
		}
		close_seven_segment();
	} 		
}


