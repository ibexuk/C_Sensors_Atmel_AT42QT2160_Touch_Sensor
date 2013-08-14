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



#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	TOUCH_C						//(Our header file define)
#include "touch-at42qt2160.h"



//**************************************
//**************************************
//********** INITIALISE TOUCH **********
//**************************************
//**************************************
//Returns 1 if initialised OK, 0 if failed
BYTE touch_init (void)
{
	BYTE data_buffer[2];
	BYTE address;
	
	TOUCH_RESET(0);
	TOUCH_PAUSE_35US;				//min 10uS to cause reset
	
	TOUCH_RESET(1);
	TOUCH_PAUSE_35US;
	
	while (TOUCH_CHANGE)							//Wait for change to go low to indicate IC ready
		;
	
	//----- READ CHIP ID -----
	//To verify we have comms to the IC
	touch_read(0, 1, &data_buffer[0]);
	if (data_buffer[0] != 0x11)
		return(0);

	//----- SLIDER CONTROL -----
	touch_write(20, ((2 << 4) | (8)));			//HYST (0 - 15) | NUM_KEYS (2 - 8)
	
	//----- SLIDER OPTIONS -----
	touch_write(21, 0);								//Slider RESOLUTION

	//----- SET BURST LENGTH -----
	//Y0 keys
	for (address = 54; address <= 61; address++)
		touch_write(address, 32);					//typical values: 0 = key disabled, 8 = low sensitivity(fastest) to 32 = high sensitivity(slowest)

	//Y1 keys
	for (address = 62; address <= 69; address++)
		touch_write(address, 0);					//typical values: 0 = key disabled, 8 = low sensitivity(fastest) to 32 = high sensitivity(slowest)


	//Flag that next read will need to send address first (after this sucessive reads do not need to as address is reset after each read).
	touch_read_send_address = 1;

	return(1);
}



//***********************************
//***********************************
//********** PROCESS TOUCH **********
//***********************************
//***********************************
void process_touch (void)
{

	//----- IF CHANGE PIN IS NOT LOW THEN NO TOUCH EVENT HAS OCCURED -----
	if (TOUCH_CHANGE)
		return;

	//----- READ SLIDER TOUCH POSITION -----
	touch_read(2, 5, &touch_status.general_status);
}



//********************************
//********************************
//********** TOUCH READ **********
//********************************
//********************************
BYTE touch_read (BYTE start_address, BYTE len, BYTE *data_buffer)
{
	BYTE count;

	//Ship address send if this was previoulsy done (sucessive reads do not need to as address is reset after each read)
	if (touch_read_send_address)
	{
		touch_read_send_address = 0;

		//----- SEND START -----
		TOUCH_I2C_IDLE_I2C;
		TOUCH_I2C_START_I2C;
		while(TOUCH_I2C_START_IN_PROGRESS_BIT)
			;
	
		//----- SEND THE I2C DEVICE ADDRESS WITH THE WRITE BIT SET -----
		TOUCH_I2C_IDLE_I2C;
		TOUCH_I2C_WRITE_BYTE(TOUCH_I2C_ADDRESS & 0xfe);			//Bit 0 low for write
		while(TOUCH_I2C_TX_IN_PROGRESS_BIT)
			;
		if(TOUCH_I2C_ACK_NOT_RECEIVED_BIT)
			goto touch_read_fail;
	
		//----- SEND REGISTER ADDRESS -----
		TOUCH_I2C_IDLE_I2C;
		TOUCH_I2C_WRITE_BYTE(start_address);
		while(TOUCH_I2C_TX_IN_PROGRESS_BIT)
			;
		if(TOUCH_I2C_ACK_NOT_RECEIVED_BIT)
			goto touch_read_fail;
	
	
		//----- SEND RE-START -----
		//TOUCH_I2C_IDLE_I2C;
		//TOUCH_I2C_RESTART_I2C;
		//while(TOUCH_I2C_RESTART_IN_PROGRESS_BIT)
		//	;
	
		//The At42QT2160 doesn't accept a re-start.  You have to send a stop, then pause then send a start for it to be ready to respond to a read.  Not documented but discovered the hard way.
	
		//----- SEND STOP -----
		TOUCH_I2C_IDLE_I2C;
		TOUCH_I2C_STOP_I2C;
		while(TOUCH_I2C_STOP_IN_PROGRESS_BIT)
			;
		
		//Pause for at least 35uS (otherwise At42QT2160 will not ack the address + read byte)
		TOUCH_PAUSE_35US;

	} //if (touch_read_send_address)


	//----- SEND START -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_START_I2C;
	while(TOUCH_I2C_START_IN_PROGRESS_BIT)
		;

	//----- SEND THE ADDRESS WITH THE READ BIT SET -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_WRITE_BYTE(TOUCH_I2C_ADDRESS | 0x01);			//Bit 0 high for read
	while(TOUCH_I2C_TX_IN_PROGRESS_BIT)
		;
	if(TOUCH_I2C_ACK_NOT_RECEIVED_BIT)
		goto touch_read_fail;


	//----- READ THE DATA BYTES -----
	while (len)
	{
		TOUCH_I2C_IDLE_I2C;
		#ifdef TOUCH_I2C_READ_BYTE_START
			TOUCH_I2C_READ_BYTE_START
		#endif
		*data_buffer++ = TOUCH_I2C_READ_BYTE;			//Get the byte

		start_address++;
		len--;

		if (len)
		{
			//----- GETTING ANOTHER BYTE - SEND ACK -----
			TOUCH_I2C_IDLE_I2C;
			TOUCH_I2C_ACK;
			while(TOUCH_I2C_ACK_IN_PROGRESS_BIT)
				;
		}
		else
		{
			//----- LAST BYTE DONE - SEND NAK -----
			TOUCH_I2C_IDLE_I2C;
			TOUCH_I2C_NOT_ACK;
			while(TOUCH_I2C_ACK_IN_PROGRESS_BIT)
				;
		}
	}

	//----- SEND STOP -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_STOP_I2C;
	while(TOUCH_I2C_STOP_IN_PROGRESS_BIT)
		;

	//----- ALL DONE - EXIT -----
	return (1);

//----- I2C COMMS FAILED -----
touch_read_fail:

	//SEND STOP
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_STOP_I2C;
	while(TOUCH_I2C_STOP_IN_PROGRESS_BIT)
		;

	return (0);
}




//*********************************
//*********************************
//********** TOUCH WRITE **********
//*********************************
//*********************************
BYTE touch_write (BYTE address, BYTE data)
{

	//----- SEND THE START CONDITION -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_START_I2C;
	while(TOUCH_I2C_START_IN_PROGRESS_BIT)
		;

	//----- SEND THE I2C DEVICE ADDRESS WITH THE WRITE BIT SET -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_WRITE_BYTE(TOUCH_I2C_ADDRESS & 0xfe);			//Bit 0 low for write
	while(TOUCH_I2C_TX_IN_PROGRESS_BIT)
		;
	if(TOUCH_I2C_ACK_NOT_RECEIVED_BIT)
		goto touch_write_fail;

	//----- SEND REGISTER ADDRESS -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_WRITE_BYTE(address);
	while(TOUCH_I2C_TX_IN_PROGRESS_BIT)
		;
	if(TOUCH_I2C_ACK_NOT_RECEIVED_BIT)
		goto touch_write_fail;

	//----- WRITE THE DATA BYTE -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_WRITE_BYTE(data);
	while(TOUCH_I2C_TX_IN_PROGRESS_BIT)
		;
	if(TOUCH_I2C_ACK_NOT_RECEIVED_BIT)
		goto touch_write_fail;

	//----- SEND STOP -----
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_STOP_I2C;
	while(TOUCH_I2C_STOP_IN_PROGRESS_BIT)
		;

	//PAUSE
	TOUCH_PAUSE_35US;

	//----- ALL DONE - EXIT -----
	return (1);

//----- I2C COMMS FAILED -----
touch_write_fail:

	//SEND STOP
	TOUCH_I2C_IDLE_I2C;
	TOUCH_I2C_STOP_I2C;
	while(TOUCH_I2C_STOP_IN_PROGRESS_BIT)
		;

	return (0);
}



