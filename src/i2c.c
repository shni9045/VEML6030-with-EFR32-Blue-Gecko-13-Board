/*
 * i2c.c - Source file Containing implementation of I2C functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#include "i2c.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG (1)

#include "src/log.h"

// VLM6030 I2C Address
#define VLM6030_I2C_ADDR (0x48)

// Measure temperature No Hold Master Mode
#define MEASURE_TEMP_CMD (0x04)

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


bool I2C_Write_VEML6030init(void){

  // Measure temperature No Hold Master Mode command
  uint8_t command;
  uint8_t commanddata[2];

  command = 0x00;
  commanddata[0] = 0x00;
  commanddata[1] = 0x20;

  I2C_TransferReturn_TypeDef check_transfer;

  // Assign address, set write flag  and pass command to buffer
  I2C_TransferSeq_TypeDef write ={

      .addr = VLM6030_I2C_ADDR<<1,
      .flags = I2C_FLAG_WRITE_WRITE,

      .buf[0].data = &command,
      .buf[0].len = 1,

      .buf[1].data = commanddata,
      .buf[1].len = 2,



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




uint16_t read_ALS_VLM6030(void){


    uint16_t light = 0;

    I2C_TransferSeq_TypeDef seq;
    I2C_TransferReturn_TypeDef ret;

    uint8_t read_data[2];
    uint8_t write_data[1];

    write_data[0] = 0x04;


    seq.addr = VLM6030_I2C_ADDR<<1;
    seq.flags = I2C_FLAG_WRITE_READ;
    seq.buf[0].data = write_data;
    seq.buf[0].len = 1;


    seq.buf[1].data = read_data;
    seq.buf[1].len = 2;


    ret = I2CSPM_Transfer(I2C0, &seq);

   // while(ret == i2cTransferNack);

    if (ret != i2cTransferDone){

        LOG_ERROR("i2c transfer failed\r");
        return 0;

    }

    // Combine 8 bit words by left shiffting MSB by 8
    light =(read_data[0])+(256*read_data[1]);

    // LOG the temperature
    LOG_INFO("Current ALS Value : %d\r",(int32_t)light);



    return 1;

}
