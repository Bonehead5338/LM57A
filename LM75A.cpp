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
						
	//return true on successful completion
	DEBUG_PRINT("LM75A Init Complete ", );
	return true;
}

bool LM75AClass::setConfiguration(uint8_t config)
{
	//send configuration
	return setRegister(LM75A_REG_CONF, config, 2);
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
	return setRegister(location, *(pData + 1), *pData);
}

bool LM75AClass::setRegister(uint8_t location, uint8_t length, uint8_t data_1, uint8_t data_2)
{
	//send register location plus 2 bytes of data
	Wire.beginTransmission(I2C_Address_7bit);
	Wire.write(location);
	if (length > 1) Wire.write(data_1);
	if (length > 2) Wire.write(data_2);

	//returns non-zero on error
	return !Wire.endTransmission(); 
}

bool LM75AClass::getRegister(uint8_t location, uint8_t length, uint8_t* result)
{
	if (!setRegister(location, 1))
	{
		DEBUG_PRINT("LM75A Get Register Failed to set pointer to selected Register ", );
		return false;
	}

	// request {length} bytes from LM75A, result is number of bytes returned
	if (Wire.requestFrom(I2C_Address_7bit, length) != length)     
	{
		DEBUG_PRINT("LM75A Read Register Failed ", );
		return false;
	}
	else
	{
		Wire.readBytes(result, length);
		return true;
	}
}

bool LM75AClass::sleep()
{
	return SleepWake(true);
}

bool LM75AClass::wake()
{
	return SleepWake(false);
}

bool LM75AClass::SleepWake(bool sleep)
{
	//select config register
	if (!setRegister(LM75A_REG_CONF, 1))
	{
		DEBUG_PRINT("LM75A Sleep/Wake Failed to set pointer to Config Register ", );
		return false;
	}

	uint8_t rx_config; //1 bytes for received config

	if (!getRegister(LM75A_REG_CONF, 1, (uint8_t*)&rx_config))
	{
		return false;
	}

	if (sleep)
	{
		//set bit 0 to enable temp shutdown
		rx_config |= 0x01;
	}
	else
	{
		//clear bit 0 to disable tempo shutdown
		rx_config &= 0xFE;
	}

	if (!setConfiguration(rx_config))
	{
		DEBUG_PRINT("LM75A Sleep/Wake Failed to Config Register ", rx_config);
		return false;
	}
	else
		return true;
}

bool LM75AClass::readTemp(int* result)
{
	if (!setRegister(LM75A_REG_TEMP, 1))
	{
		DEBUG_PRINT("LM75A Read Failed to set pointer to Temp Register ", );
		return false;
	}
	
	uint16_t RxData; //2 bytes for received value

	if (!getRegister(LM75A_REG_TEMP, 2, (uint8_t*)&RxData))
	{
		return false;
	}
		
	uint16_t rx_result;
	
	//swap bytes and store in temporary result
	uint8_t* ptr = (uint8_t*)&rx_result;

	*ptr = *((uint8_t*)&RxData + 1);
	*(ptr + 1) = *(uint8_t*)&RxData;

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

