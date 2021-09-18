/*
 * i2c.c - Source file Containing implementation of I2C functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#include "i2c.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG (1)

#include "src/log.h"

// Si7021 I2C Address
#define SI70_I2C_ADDR (0x40)

// Measure temperature No Hold Master Mode
#define MEASURE_TEMP_CMD (0xF3)

void I2C_init(void){

  uint32_t i2c_freq=0;

  // Assign values for I2C init
  I2CSPM_Init_TypeDef i2cspm = {

      .i2cClhr = i2cClockHLRStandard,
      .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
      .i2cRefFreq = 0,
      .port = I2C0,
      .portLocationScl = 14u,
      .portLocationSda = 16u,
      .sclPin = 10u,
      .sclPort = gpioPortC,
      .sdaPin = 11u,
      .sdaPort = gpioPortC,

      };

  // Initialize I2C0
  I2CSPM_Init(&i2cspm);

  i2c_freq=I2C_BusFreqGet(I2C0);


}


uint8_t* I2C_Read_Si7021(void){

  // Allocate Memory for read buffer to store temperature data
  uint8_t *temp_data=(uint8_t*)malloc(sizeof(uint8_t)*2);

  I2C_TransferReturn_TypeDef check_transfer;

  // Assign address, set read flag and assign buffer
  I2C_TransferSeq_TypeDef read = {

      .addr = SI70_I2C_ADDR<<1,
      .flags = I2C_FLAG_READ,
      .buf[0].data = temp_data,
      .buf[0].len = sizeof(temp_data),


  };

  // Perform I2C transfer
  check_transfer=I2CSPM_Transfer(I2C0,&read);

  // Wait till No acknowledgement is received
  while(check_transfer == i2cTransferNack);

  // Return temperature
  return temp_data;

}


bool I2C_Write_Si7021(void){

  // Measure temperature No Hold Master Mode command
  uint8_t command = MEASURE_TEMP_CMD;

  I2C_TransferReturn_TypeDef check_transfer;

  // Assign address, set write flag  and pass command to buffer
  I2C_TransferSeq_TypeDef write ={

      .addr = SI70_I2C_ADDR<<1,
      .flags = I2C_FLAG_WRITE,

      .buf[0].data = &command,
      .buf[0].len = 1,

  };

  // Perform I2C transfer
  check_transfer=I2CSPM_Transfer(I2C0,&write);

  // Map Return status of I2C transfer
  switch(check_transfer){

    case i2cTransferInProgress:{
      LOG_INFO("\n\rTransfer In Progress");
      break;
    }

    case i2cTransferDone:{

      return true;
      break;

    }

    case i2cTransferNack:{
      LOG_ERROR("\n\rNACK Received");
      break;
    }

    case i2cTransferBusErr:{
      LOG_ERROR("\n\rBus Error");
       break;
     }

    case i2cTransferArbLost:{

      LOG_ERROR("\n\rArbitration lost");
        break;
      }

    case i2cTransferUsageFault:{

      LOG_ERROR("\n\rUsage Fault");
        break;
      }

    case i2cTransferSwFault:{

      LOG_ERROR("\n\rSw Fault");
      break;
      }

    default:{

      break;
    }

  }

  return false;

}


void Enable_si7021(bool state){

  // Set Enable Pin high
  if (state == true){

      GPIO_PinOutSet(gpioPortD,15);

  }

  // Set Enable Pin low
  else if (state == false){

      GPIO_PinOutClear(gpioPortD,15);

  }


}

uint16_t read_temp_si7021(void){

  // Temporary pointer to point to buffer and read from
  uint8_t *temp_d;

  uint16_t temp = 0;

  uint16_t celsius = 0;

  GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, false);

  // Enable Si7021 by setting its enable signal high
  Enable_si7021(true);

  // Wait for Power up time
  timerWaitUs(80000);

  // Check if write is successful
  if(I2C_Write_Si7021() == true){

      // Wait for specified time
      timerWaitUs(10800);

      temp_d = I2C_Read_Si7021();

      // Disable si7021
      Enable_si7021(false);

      // Combine 8 bit words by left shiffting MSB by 8
      temp=(256*temp_d[0])+temp_d[1];

     // Convert Temperarure Code to degree Celsius
      celsius = ((175.72*(temp))/65535)  - 46.85;

      // Free allocated buffer
      free(temp_d);

      // LOG the temperature
      LOG_INFO("Current Temperature : %d\r",(int32_t)celsius);

      // Return temperature after converting temp code to celsius
      return (celsius);

  }

  return 0;

}


