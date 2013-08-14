/*
IBEX UK LTD http://www.ibexuk.com
Electronic Product Design Specialists
RELEASED SOFTWARE

The MIT License (MIT)

Copyright (c) 2013, IBEX UK Ltd, http://ibexuk.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//Project Name:		ATMEL AT42QT2160 TOUCH SENSOR DRIVER




//*************************************
//*************************************
//********** DRIVER REVISION **********
//*************************************
//*************************************
//
//V1.00
//- Original release






//############################
//############################
//##### USING THE DRIVER #####
//############################
//############################

//Include this header file in any .c files within your project from which you wish to use it's functions.

//##### IN YOUR INITIALISATION #####
/*
	//----- INITIALISE TOUCH SENSOR -----
	touch_init();
*/
//##### IN YOUR MAIN LOOP #####
/*
	//----- PROCESS TOUCH SENSOR -----
	process_touch();
*/



//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef TOUCH_C_INIT		//(Do only once)
#define	TOUCH_C_INIT


#define	TOUCH_I2C_ADDRESS			0x1a			//0x0d << 1 - 0x1a


//----- IO -----
#define	TOUCH_RESET(a)			LATCbits.LATC0 = a		//Reset output pin
#define	TOUCH_CHANGE			PORTBbits.RB0			//Change input

//----- DELAY -----
//Define to pause execution for at least 35uS
#include <delays.h>												//PIC18 C18 compiler delays library
#define	TOUCH_PAUSE_35US					Delay10TCYx(35)		//PIC18 C18 compiler delays function



//------ SELECT COMPILER ------
//(Enable one of these only)
#define	TOUCH_USING_PIC18
//#define	TOUCH_USING_PIC24
//#define	TOUCH_USING_PIC32


#ifdef TOUCH_USING_PIC18
//##################
//##### PIC 18 #####
//##################

#include <i2c.h>

#define	TOUCH_I2C_START_I2C					StartI2C1					//Generate bus start condition
#define	TOUCH_I2C_START_IN_PROGRESS_BIT		SSP1CON2bits.SEN			//Bit indicating start is still in progress
//#define	TOUCH_I2C_RESTART_I2C				RestartI2C1					//Generate bus restart condition
//#define	TOUCH_I2C_RESTART_IN_PROGRESS_BIT	SSP1CON2bits.RSEN			//Bit indicating re-start is still in progress
#define	TOUCH_I2C_STOP_I2C					StopI2C1					//Generate bus stop condition
#define	TOUCH_I2C_STOP_IN_PROGRESS_BIT		SSP1CON2bits.PEN			//Bit indicating Stop is still in progress
#define	TOUCH_I2C_WRITE_BYTE(a)				WriteI2C1(a)				//Write byte to I2C device
#define	TOUCH_I2C_TX_IN_PROGRESS_BIT		SSP1STATbits.R_W			//Bit indicating transmit byte is still in progress
#define	TOUCH_I2C_ACK_NOT_RECEIVED_BIT		SSP1CON2bits.ACKSTAT		//Bit that is high when ACK was not received
//#define	TOUCH_I2C_READ_BYTE_START										//Read byte from I2C device function (optional)
#define	TOUCH_I2C_READ_BYTE					ReadI2C1()					//Read byte from I2C device function / result byte of TOUCH_I2C_READ_FUNCTION_START
#define TOUCH_I2C_ACK						AckI2C1						//Generate bus ACK condition
#define TOUCH_I2C_NOT_ACK					NotAckI2C1					//Generate bus Not ACK condition
#define	TOUCH_I2C_ACK_IN_PROGRESS_BIT		SSP1CON2bits.ACKEN			//Bit indicating ACK is still in progress
#define	TOUCH_I2C_IDLE_I2C					IdleI2C1					//Test if I2C1 module is idle (wait until it is ready for next operation)

#endif //#ifdef TOUCH_USING_PIC18


#ifdef TOUCH_USING_PIC24
//##################
//##### PIC 24 #####
//##################
#define	TOUCH_I2C_START_I2C					I2C2CONbits.SEN = 1										//Generate bus start condition
#define	TOUCH_I2C_START_IN_PROGRESS_BIT		I2C2CONbits.SEN											//Bit indicating start is still in progress
//#define	TOUCH_I2C_RESTART_I2C				I2C2CONbits.RSEN = 1									//Generate bus restart condition
//#define	TOUCH_I2C_RESTART_IN_PROGRESS_BIT	I2C2CONbits.RSEN										//Bit indicating re-start is still in progress
#define	TOUCH_I2C_STOP_I2C					I2C2CONbits.PEN = 1										//Generate bus stop condition
#define	TOUCH_I2C_STOP_IN_PROGRESS_BIT		I2C2CONbits.PEN											//Bit indicating Stop is still in progress
#define	TOUCH_I2C_WRITE_BYTE(a)				I2C2TRN = a												//Write byte to I2C device
#define	TOUCH_I2C_TX_IN_PROGRESS_BIT		I2C2STATbits.TRSTAT										//Bit indicating transmit byte is still in progress
#define	TOUCH_I2C_ACK_NOT_RECEIVED_BIT		I2C2STATbits.ACKSTAT									//Bit that is high when ACK was not received
#define	TOUCH_I2C_READ_BYTE_START			I2C2CONbits.RCEN = 1; while(I2C2STATbits.RBF == 0) ;	//Read byte from I2C device function (optional)
#define	TOUCH_I2C_READ_BYTE					I2C2RCV													//Read byte from I2C device function / result byte of TOUCH_I2C_READ_FUNCTION_START
#define TOUCH_I2C_ACK						I2C2CONbits.ACKDT = 0; I2C2CONbits.ACKEN = 1			//Generate bus ACK condition
#define TOUCH_I2C_NOT_ACK					I2C2CONbits.ACKDT = 1; I2C2CONbits.ACKEN = 1			//Generate bus Not ACK condition
#define	TOUCH_I2C_ACK_IN_PROGRESS_BIT		I2C2CONbits.ACKEN										//Bit indicating ACK is still in progress
#define	TOUCH_I2C_IDLE_I2C					while ((I2C2CON & 0x001F) | (I2C2STATbits.R_W))			//Test if I2C1 module is idle (wait until it is ready for next operation)

#endif //#ifdef TOUCH_USING_PIC24


#ifdef TOUCH_USING_PIC32
//##################
//##### PIC 32 #####
//##################
#define	TOUCH_I2C_START_I2C					StartI2C1					//Generate bus start condition
#define	TOUCH_I2C_START_IN_PROGRESS_BIT		I2C1CONbits.SEN				//Bit indicating start is still in progress
//#define	TOUCH_I2C_RESTART_I2C				RestartI2C1					//Generate bus restart condition
//#define	TOUCH_I2C_RESTART_IN_PROGRESS_BIT	I2C1CONbits.RSEN			//Bit indicating re-start is still in progress
#define	TOUCH_I2C_STOP_I2C					StopI2C1					//Generate bus stop condition
#define	TOUCH_I2C_STOP_IN_PROGRESS_BIT		I2C1CONbits.PEN				//Bit indicating Stop is still in progress
#define	TOUCH_I2C_WRITE_BYTE(a)				MasterWriteI2C1(a)			//Write byte to I2C device
#define	TOUCH_I2C_TX_IN_PROGRESS_BIT		I2C1STATbits.TRSTAT			//Bit indicating transmit byte is still in progress
#define	TOUCH_I2C_ACK_NOT_RECEIVED_BIT		I2C1STATbits.ACKSTAT		//Bit that is high when ACK was not received
//#define	TOUCH_I2C_READ_BYTE_START										//Read byte from I2C device function (optional)
#define	TOUCH_I2C_READ_BYTE					MasterReadI2C1()			//Read byte from I2C device function / result byte of TOUCH_I2C_READ_FUNCTION_START
#define TOUCH_I2C_ACK						AckI2C1						//Generate bus ACK condition
#define TOUCH_I2C_NOT_ACK					NotAckI2C1					//Generate bus Not ACK condition
#define	TOUCH_I2C_ACK_IN_PROGRESS_BIT		I2C1CONbits.ACKEN			//Bit indicating ACK is still in progress
#define	TOUCH_I2C_IDLE_I2C					IdleI2C1					//Test if I2C1 module is idle (wait until it is ready for next operation)

#endif //#ifdef TOUCH_USING_PIC32







#endif





//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef TOUCH_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
BYTE touch_read (BYTE start_address, BYTE len, BYTE *data_buffer);
BYTE touch_write (BYTE address, BYTE data);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE touch_init (void);
void process_touch (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE touch_init (void);
extern void process_touch (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef TOUCH_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE touch_read_send_address;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)



struct TOUCH_STATUS
{
	BYTE general_status;
	BYTE key_status_1;
	BYTE key_status_2;
	BYTE slider_position;
	BYTE gpio;
} touch_status;

#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------



extern struct TOUCH_STATUS
{
	BYTE general_status;
	BYTE key_status_1;
	BYTE key_status_2;
	BYTE slider_position;
	BYTE gpio;
} touch_status;

#endif







