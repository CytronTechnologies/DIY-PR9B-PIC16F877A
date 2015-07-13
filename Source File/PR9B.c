//=======================================================================================
//	Author				:Cytron Technologies
//	Project				:DIY Project (PR9- Keypad door security)
//	Project description             :PIC18F877A + 4x4 keypad + LCD are used to build a keypad door
//                                   security system which will activate the relay and buzzer after
//                                   a preset 6-digit password is entered.
//                                   LCD will display ****** when keypad is pressed.
//                                   preset password for this program is 123456.Compatible with
//                                   -MPLAB IDE with HITECH C compiler
//                                   -MPLABX with HITECH C compiler v9.83/v9.82/v9.80 and XC8 compiler.
//
//========================================================================================


//========================================================================================
#if defined(__XC8)
  #include <xc.h>
 #pragma config CONFIG = 0x3F32
//FOSC = EXTRC     // Oscillator Selection bits (RC oscillator)
//WDTE = ON        // Watchdog Timer Enable bit (WDT enabled)
//PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
//BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
//LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
//CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
//WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
//CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#else
//#include <htc.h>                  //include the PIC microchip header file
#include <pic.h>

//	configuration
//==========================================================================
__CONFIG (0x3F32);

#endif

//==========================================================================================
//	define
//==========================================================================================
#define	RS                      RC0
#define	E                       RC1
#define	LED_RED                 RC2
#define LED_YELLOW              RC3
#define	LCD_DATA                PORTD
#define RELAY                   RB1
#define BUZZER                  RB2

//===========================================================================================
//	function prototype
//===========================================================================================
void delay(unsigned long data);
void send_config(unsigned char data);
void send_char(unsigned char data);
void e_pulse(void);
void lcd_goto(unsigned char data);
void lcd_clr(void);
void send_string(const char *s);
void clearrow1(void);
void clearrow2(void);
void clearrow3(void);
void clearrow4(void);
void scancolumn1(void);
void scancolumn2(void);
void scancolumn3(void);
void scancolumn4(void);
void beep_once(void);
void beep_twice(void);

//============================================================================================
//	global variable
//============================================================================================
unsigned char password_count=0;
unsigned char keyin_char[6];              // Declare an array to stall the 6-digit key in password
unsigned char stalled_char[6]="123456";		// Declare an array to stall the 6-digit desired password

//============================================================================================
//	main function
//============================================================================================
void main(void)
{
	ADCON1=0b00000110;	//set all portA pins as digital I/O
	TRISA=0b11001111;   //clear bit 4&5 portA as output and set the rest as input
	TRISB=0b00000000;   //set portB as output
	TRISD=0b00000000;   //set portD as output
	TRISC=0b11110000;   //set bit4-7 portC as input(connected to 4 row of keypad)
	TRISE=0b00000000; 	//set portE as output

	PORTC=0;
	PORTD=0;
	RELAY=0;
	BUZZER=0;
	LED_YELLOW=0;
	LED_RED=0;

	send_config(0b00001001);                //clear display at lcd
	send_config(0b00000010);                //Lcd Return to home
	send_config(0b00000110);                //entry mode-cursor increase 1
	send_config(0b00001100);                //diplay on, cursor off and cursor blink off
	send_config(0b00111000);                //function
	
	lcd_clr();                          //clear LCD
	delay(1000);                        //delay
	lcd_goto(0);                        //initial display
	send_string("PLEASE ENTER");        //Display "PLEASE ENTER" on lcd
	lcd_goto(20);                       //Display on 2nd line
	send_string("6-DIGIT PASSWORD");    //Display "6-DIGIT PASSWORD" on lcd

	
	while(1)
	{                   //keypad scanning algorithm
			clearrow1();		//Clear 1st output pin and set the others
			scancolumn1();	//scan column 1-4
			clearrow2();		//Clear 2nd output pin and set the others
			scancolumn2();	//scan column
			clearrow3();		//Clear 3rd output pin and set the others
			scancolumn3();	//scan column
			clearrow4();		//Clear 4th output pin and set the others
			scancolumn4();	//scan column
			
	if(password_count==6)	
	{
            password_count=0;
            if((keyin_char[0]==stalled_char[0])&&(keyin_char[1]==stalled_char[1])&&
                (keyin_char[2]==stalled_char[2])&&(keyin_char[3]==stalled_char[3])&&
		(keyin_char[4]==stalled_char[4])&&(keyin_char[5]==stalled_char[5]))	//compare the keyin value with stalled value to test whether password is correct
                    {
                        lcd_clr();				//clear lcd
			lcd_goto(0);
			send_string("SUCCESS!");		//display SUCCESS
			LED_YELLOW=1;               //yellow light on
			RELAY=1;                    //relay on
			beep_once();                //beep one time
			while(1);                   //infinity loop
				}
	else
	{
            lcd_clr();						//clear lcd
            lcd_goto(0);
            send_string("ERROR!");//display ERROR!
            LED_RED=1;						//red light on
            beep_twice();					//beep two time
            while(1);             //infinity loop
                        }
			}
	}
}

//=======================================================================================
//	scanning functions
//=======================================================================================
void clearrow1(void)			//clear the 1st row and set the others
{
	RE1=0;                	//RE1,RE0, RA5 and RA4 are the output pins from PIC which connect to 4 pins of keypad
	RE0=1;
	RA5=1;
	RA4=1;				
}

void clearrow2(void)			//clear the 2nd row and set the others
{
	RE1=1;								
	RE0=0;
	RA5=1;
	RA4=1;				
}

void clearrow3(void)			//clear the 3rd row and set the others
{
	RE1=1;								
	RE0=1;
	RA5=0;
	RA4=1;				
}

void clearrow4(void)			//clear the 4th roe and set the others
{
	RE1=1;								
	RE0=1;
	RA5=1;
	RA4=0;				
}

void scancolumn1(void)
{
	if(RA0==0)                                //if key '1' is being pressed
	{
		while(RA0==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='1';         //Stall the '1' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA1==0)                           //if key '2' is being pressed
	{
		while(RA1==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='2';         //Stall the '2' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA2==0)                           //if key '3' is being pressed
	{
		while(RA2==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='3';         //Stall the '3' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA3==0)                           //if key 'A' is being pressed
	{
		while(RA3==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='A';         //Stall the 'A' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
}

void scancolumn2(void)
{
	if(RA0==0)                                //if key '4' is being pressed
	{
		while(RA0==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='4';         //Stall the '4' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA1==0)                           //if key '5' is being pressed
	{
		while(RA1==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='5';         //Stall the '5' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA2==0)                           //if key '6' is being pressed
	{
		while(RA2==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='6';         //Stall the '6' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA3==0)                           //if key 'B' is being pressed
	{
		while(RA3==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='B';         //Stall the 'B' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
}

void scancolumn3(void)
{
	if(RA0==0)                                //if key '7' is being pressed
	{
		while(RA0==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='7';         //Stall the '7' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA1==0)                           //if key '8' is being pressed
	{
		while(RA1==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='8';         //Stall the '8' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA2==0)                           //if key '9' is being pressed
	{
		while(RA2==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='9';         //Stall the '9' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA3==0)                           //if key 'C' is being pressed
	{
		while(RA3==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='C';         //Stall the 'C' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
}

void scancolumn4(void)
{
	if(RA0==0)                                //if key '*' is being pressed
	{
		while(RA0==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='*';         //Stall the '*' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA1==0)                           //if key '0' is being pressed
	{
		while(RA1==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='0';         //Stall the '0' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA2==0)                           //if key '#' is being pressed
	{
		while(RA2==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='#';         //Stall the '#' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}
	else if(RA3==0)                           //if key 'D' is being pressed
	{
		while(RA3==0)continue;                  //waiting the key to be released
		if(password_count==0)lcd_clr();         //Clear the LCD if the key is the 1st password
		lcd_goto(password_count);               //The cursor of LCD points to the column equivalent to the value of password_count variable
		send_char('*');                         //Display the symbol '*' at LCD
		keyin_char[password_count]='D';         //Stall the 'D' value at the keyin_char array
		password_count+=1;                      //increase the Password_count variable's value by 1 and the result stall back to the variable
	}	
}

//===============================================================================================
//	General Purpose	functions
//===============================================================================================
void delay(unsigned long data)
{
	for( ;data>0;data-=1);
}

void beep_once(void)
{
	BUZZER=1;					//buzzer on
	delay(8000);
	BUZZER=0;					//buzzer off
}

void beep_twice(void)
{
	BUZZER=1;					//buzzer on
	delay(8000);
	BUZZER=0;					//buzzer off
	delay(13000);
	BUZZER=1;					//buzzer on
	delay(8000);
	BUZZER=0;					//buzzer off
}

//========================================================================================
//	LCD	functions
//========================================================================================
void send_config(unsigned char data)
{
	RS=0;						//clear rs into config mode
	LCD_DATA=data;
	delay(50);
	e_pulse();
}

void send_char(unsigned char data)
{
	RS=1;						//set rs into write mode
	LCD_DATA=data;
	delay(50);
	e_pulse();
}

void e_pulse(void)
{
	E=1;
	delay(50);
	E=0;
	delay(50);
}

void lcd_goto(unsigned char data)
{
 	if(data<16)
	{
	 	send_config(0x80+data);
	}
	else
	{
	 	data=data-20;
		send_config(0xc0+data);
	}
}

void lcd_clr(void)
{
 	send_config(0x01);
	delay(50);	
}

void send_string(const char *s)
{          
	unsigned char i=0;
  	while (s && *s)send_char (*s++);

}

