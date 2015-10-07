// 
// 
// 

#include "LM75A.h"

bool LM75AClass::init(uint8_t address, float Tos, float Thyst, uint8_t config)
{
	DEBUG_PRINT("LM75A Begin Init ", );

	//verify valid address adn create 7-bit full address
	if (address >= 0 && address < 128)
		I2C_Address_7bit = LM75A_FIXED_ADDR | address;
	else
	{
		DEBUG_PRINT("LM75A Init Invalid Address: ", address);
		return false;
	}

	//if non-default config, send config
	if (config != 0)
	{
		//check for valid config (sig 3 bits = 0) before sending
		if (!(config & 0xE0))
		{
			if (!setConfiguration(config))
			{
				DEBUG_PRINT("LM75A Init Failed to set config: ", config);
				return false;
			}			
		}
		else
		{
			DEBUG_PRINT("LM75A Init Config Invalid: ", config);
			return false;
		}
	}

	//if non-default Tos, send Tos
	if (Tos != 80)
		if (!setSetpoint(Tos, LM57A_REG_TOS))
		{
			DEBUG_PRINT("LM75A Init Failed to set Tos: ", Tos);
			return false;
		}

	//if non-default Thyst, send Thyst
	if (Thyst != 75)
		if (!setSetpoint(Thyst, LM57A_REG_THYST))
		{
			DEBUG_PRINT("LM57A Init Failed to set Thyst ", Thyst);
			return false;
		}
		
	//set pointer to Temperature register
	Wire.beginTransmission(I2C_Address_7bit);
	Wire.write(LM75A_REG_TEMP);
	if (Wire.endTransmission())
	{
		DEBUG_PRINT("LM75A Init Failed to select Temperature Register: ", LM75A_REG_TEMP);
		return false;	//returns non-zero on error
	}
						
	//return true on successful completion
	DEBUG_PRINT("LM75A Init Complete ", );
	return true;
}

bool LM75AClass::setConfiguration(uint8_t config)
{
	//send configuration
	Wire.beginTransmission(I2C_Address_7bit);
	Wire.write(LM75A_REG_CONF);
	Wire.write(config);	
	
	return !Wire.endTransmission(); //returns non-zero on error
}


bool LM75AClass::setSetpoint(float setpoint, uint8_t location)
{
	//check for valid location
	if (location != LM57A_REG_TOS && location != LM57A_REG_THYST)
		return false;
	
	//check for valid range
	if (setpoint < LM57A_T_MIN || setpoint > LM57A_T_MAX)
		return false;

	// * 128 = 7-bit shift to left of non-sign data
	int shifted_int = (int)(2.0 * setpoint * 128.0);

	//8-bit unsigned pointer to shifted int
	uint8_t* pData = (uint8_t*)&shifted_int;

	//send configuration (swap MSB and LSB)
	Wire.beginTransmission(I2C_Address_7bit);
	Wire.write(location);
	Wire.write(*(pData + 1));
	Wire.write(*pData);

	return !Wire.endTransmission(); //returns non-zero on error
}

bool LM75AClass::readTemp(int* result)
{
	uint8_t RxData[2]; //2 bytes for received value

	if (Wire.requestFrom(I2C_Address_7bit, (uint8_t)2) != 2)     // request 2 bytes from LM75A, result is number of bytes returned
	{
		DEBUG_PRINT("LM75A Read Temp Failed ", );
		return false;
	}
	else
	{
		Wire.readBytes(RxData, 2);
	}
	
	uint16_t rx_result;
	
	//swap bytes and store in temporary result
	uint8_t* ptr = (uint8_t*)&rx_result;

	*ptr = RxData[1];
	*(ptr + 1) = RxData[0];

	//shift data 5 places to the right (left-align data)
	rx_result >>= 5;

	//detect and relocate negative indication bit (11th bit [10] to 16th bit [15])
	if (rx_result & 0x0400)
	{
		//strip negative bit at 11th position
		rx_result &= 0x03FF;
		//set bit at 16th postion
		rx_result |= 0x8000;
	}

	//convert to int (signed) and store in result
	*result = *(int*)&rx_result;

	return true;
}

int LM75AClass::readCelcius()
{
	return round(readCelciusFloat());
}

float LM75AClass::readCelciusFloat()
{
	int RxTemp;
	if (readTemp(&RxTemp))
	{
		return (float)RxTemp * 0.125;
	}
	else
		return 999.0;	//invalid value
}

int LM75AClass::readFarenheit()
{
	return round(readFarenheitFloat());
}

float LM75AClass::readFarenheitFloat()
{
	int RxTemp;
	if (readTemp(&RxTemp))
	{
		float TempC =  (float)RxTemp * 0.125;
		float TempF = (TempC * (9.0 / 5.0)) + 32;
		return TempF;
	}
	else
		return 999.0;	//invalid value
}


LM75AClass LM75A;

