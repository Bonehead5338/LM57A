// LM75A.h

#ifndef _LM75A_h
#define _LM75A_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//include wire library for I2C operation
#include "Wire.h"

//compile-time debug option
//#define LM57A_DEBUG

#ifdef LM57A_DEBUG
#define DEBUG_PRINT(comment, value)	{	Serial.print(comment);	\
										Serial.println(value);		\
									}
#else
#define DEBUG_PRINT(comment, value)
#endif

//fixed portion of I2C address
#define LM75A_FIXED_ADDR 0x48

//register pointers
#define LM75A_REG_TEMP	0
#define LM75A_REG_CONF	1
#define LM57A_REG_THYST	2
#define LM57A_REG_TOS	3

//min and max for setpoints
#define LM57A_T_MIN -128.0
#define LM57A_T_MAX	127.5

class LM75AClass
{
 protected:
	 
	 //full 7-bit address of LM75A
	 uint8_t I2C_Address_7bit;

	 //Tos and Thyst as ints
	 int Tos;
	 int Thyst;

	 //set register pointer
	 bool setCurrentPointer(uint8_t register);
	 
	 //set configuration methods
	 bool setConfiguration(uint8_t config);
	 bool setSetpoint(float setpoint, uint8_t location);
	 
	 //read temperature (raw)
	 bool readTemp(int* result);
 
 public:
	
	//initialization
	bool init(uint8_t address, float Tos = 80.0, float Thyst = 75.0, uint8_t config = 0);

	//read temperature
	float readCelciusFloat();
	int readCelcius();
	float readFarenheitFloat();
	int readFarenheit();
		
};

extern LM75AClass LM75A;

#endif

