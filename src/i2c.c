/*
 * i2c.c - Source file Containing implementation of I2C functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#include "i2c.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG (1)

#include "src/log.h"

// CCS811 I2C Address
#define CCS811_I2C_ADDR (0xB6)

#define CCS811_ADDR_ALG_RESULT_DATA (0x02)

#define CCS811_ADDR_APP_START (0xF4)

#define CCS811_ADDR_STATUS (0x00)

#define CCS811_ADDR_HW_ID (0x20)

#define CCS811_ADDR_MEASURE_MODE (0x01)

#define CCS811_HW_ID (0x81)



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

void Enable_CCS811(bool state){

  // Set Enable Pin high
  if (state == true){

      GPIO_PinOutSet(gpioPortA,3);

  }

  // Set Enable Pin low
  else if (state == false){

      GPIO_PinOutClear(gpioPortA,3);

  }


}

void Wake_CCS811(bool state){

  // Set Enable Pin high
  if (state == true){

      //GPIO_PinOutSet(gpioPortD,10);
      //GPIO_PinOutClear(gpioPortA,2);
      GPIO_PinOutSet(gpioPortA,2);

  }

  // Set Enable Pin low
  else if (state == false){

      //GPIO_PinOutClear(gpioPortD,10);
      //GPIO_PinOutSet(gpioPortA,2);
      GPIO_PinOutClear(gpioPortA,2);

  }


}



uint32_t init_CCS811(void){

  uint8_t id;
  uint32_t check;

  //Enable_CCS811(true);
  Wake_CCS811(true);


  // Wait for Power up time
  timerWaitUs(100000);


  check  =  readMailbox_CCS811(CCS811_ADDR_HW_ID, 1, &id);

  LOG_INFO("HARDWARE ID : %02X\r",id);

  if (check != 1 && id != CCS811_HW_ID){

      LOG_ERROR("Initialize CCS811 failed\r");
      return 0;

  }


  Wake_CCS811(false);

  return 1;


}

uint32_t readMailbox_CCS811(uint8_t id, uint8_t length, uint8_t *data){

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t write_data[1];


  Wake_CCS811(true);

  write_data[0] = id;


  seq.addr = CCS811_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE_READ;
  seq.buf[0].data = write_data;
  seq.buf[0].len = 1;


  seq.buf[1].data = data;
  seq.buf[1].len = length;


  ret = I2CSPM_Transfer(I2C0, &seq);

  if (ret != i2cTransferDone){

      LOG_ERROR("i2c transfer failed\r");
      return 0;

  }


  Wake_CCS811(false);

  return 1;

}

uint32_t setappmode_CCS811(void){

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t read_data[2];
  uint8_t write_data[1];

  Wake_CCS811(true);

  write_data[0] = CCS811_ADDR_APP_START;

  seq.addr = CCS811_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE;

  seq.buf[0].data = write_data;
  seq.buf[0].len = 1;

  seq.buf[1].data = read_data;
  seq.buf[1].len = 0;


  ret = I2CSPM_Transfer(I2C0, &seq);

   if (ret != i2cTransferDone){

       LOG_ERROR("i2c transfer failed\r");
       return 0;

   }


   Wake_CCS811(false);

   return 1;

}

uint32_t startapp_CCS811(void){

  uint32_t result;
  uint8_t status;

  result  = readMailbox_CCS811(CCS811_ADDR_STATUS, 1, &status);

  if ((status & 0x10 ) != 0x10){

    LOG_ERROR("Application Missing\r");
    return 0;

  }

  result += setappmode_CCS811();

  result = readMailbox_CCS811(CCS811_ADDR_STATUS,1,&status);

  if ((status & 0x90 ) != 0x90){

      LOG_ERROR("Error in setting Application Mode\r");
      return 0;
  }


  return 1;


}


uint32_t setMode_CCS811(uint8_t mode){

    I2C_TransferSeq_TypeDef seq;
    I2C_TransferReturn_TypeDef ret;

    uint8_t read_data[1];
    uint8_t write_data[2];

    Wake_CCS811(true);

    mode = (mode & 0x38);

    write_data[0] = CCS811_ADDR_MEASURE_MODE;

    write_data[1] = mode;

    seq.addr = CCS811_I2C_ADDR;
    seq.flags = I2C_FLAG_WRITE;

    seq.buf[0].data = write_data;
    seq.buf[0].len = 2;

    seq.buf[1].data = read_data;
    seq.buf[1].len = 0;


    ret = I2CSPM_Transfer(I2C0, &seq);

     if (ret != i2cTransferDone){

         LOG_ERROR("i2c transfer failed\r");
         return 0;

     }


     Wake_CCS811(false);

     return 1;


}



uint32_t measurequality_CCS811(uint16_t *eco2,uint16_t *tvoc){

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t read_data[4];
  uint8_t write_data[1];

  *eco2 = 0;
  *tvoc = 0;

  Wake_CCS811(true);

  write_data[0] = CCS811_ADDR_ALG_RESULT_DATA;

   seq.addr = CCS811_I2C_ADDR;
   seq.flags = I2C_FLAG_WRITE;

   seq.buf[0].data = write_data;
   seq.buf[0].len = 1;

   seq.buf[1].data = read_data;
   seq.buf[1].len = 4;

   ret = I2CSPM_Transfer(I2C0, &seq);

   if (ret != i2cTransferDone){

       LOG_ERROR("i2c transfer failed\r");
       return 0;

   }

   *eco2 = ((uint16_t ) read_data[0] << 8 ) + (uint16_t) read_data[1];
   *tvoc = ((uint16_t ) read_data[2] << 8 ) + (uint16_t) read_data[3];

   Wake_CCS811(false);

   return 1;


}

bool dataavailaible( void ){

  bool state = false;
  uint32_t status;
  uint8_t reg;

  uint32_t status2;
  uint8_t reg2;

  status = readMailbox_CCS811(0x00, 1, &reg);

  if ((status == 1) && ((reg & 0x08) == 0x08)){

      state = true;

  }

  else if ((status == 1) && ((reg & 0x01) == 0x01)){

      LOG_ERROR("ERROR OCCURED in reading status register\r");

      status2 = readMailbox_CCS811(0xe0, 1, &reg2);

      if (status2){

          LOG_INFO("ERROR REGISTER : %02X\r",reg2);

      }

  }

  return state;

}


