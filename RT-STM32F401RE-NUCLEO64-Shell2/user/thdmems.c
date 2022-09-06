#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "lsm6dsl.h"


// SCL_PIN     GPIOB, 8
// SDA_PIN     GPIOB, 9



#define MAX_AXIS_NUMBER     3U

uint32_t i;

/* Array per immagazzinare i dati  */
  static float cooked[MAX_AXIS_NUMBER];

/* Stringa per identificare gli assi. */
  static char axis_id[MAX_AXIS_NUMBER] = {'X', 'Y', 'Z'};



/* Configurazione standard per i mems */
  static const I2CConfig i2ccfg = {OPMODE_I2C, 400000, FAST_DUTY_CYCLE_2};


/* definizione del puntatore al'interfaccia seriale */
  static BaseSequentialStream *chp = (BaseSequentialStream*) &SD2;

/* Configurazione del driver per l'imu iNEMO */

  static LSM6DSLDriver LSM6DSLD1;

  static const LSM6DSLConfig lsm6dslcfg = { &I2CD1, &i2ccfg, LSM6DSL_SAD_VCC, NULL,
                                          NULL, LSM6DSL_ACC_FS_2G,
                                          LSM6DSL_ACC_ODR_416Hz, NULL, NULL, LSM6DSL_GYRO_FS_250DPS,
                                          LSM6DSL_GYRO_ODR_104Hz};

/* Funzione per leggere i dati dell IMU   */

 msg_t readRegister(uint8_t register_addres, uint8_t *out){
     return i2cMasterTransmitTimeout(&I2CD1, LSM6DSL_SAD_VCC, &register_addres, 1, out, 1, TIME_INFINITE);
}


/*dichiarazione thread MEMS*/
 THD_FUNCTION(thdMems, arg) {
   (void)arg;
   /* configurazione pin per l'i2c */
   palSetLineMode( LINE_ARD_D15,
                   PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN
   );

   palSetLineMode(LINE_ARD_D14,
                  PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN
   );

   /* Attivazione Seriale 2 */
   sdStart(&SD2,NULL);

   /* inizializzazione driver imu */
   lsm6dslObjectInit(&LSM6DSLD1);

   /* Attivazione driver imu */
   lsm6dslStart(&LSM6DSLD1, &lsm6dslcfg);

   volatile unsigned retval = 0 ;

   while (true){

    lsm6dslAccelerometerReadCooked(&LSM6DSLD1, cooked);

     chprintf(chp, "LSM6DSL Accelerometer cooked data...\r\n");
     for(i = 0; i < LSM6DSL_ACC_NUMBER_OF_AXES; i++) {
       chprintf(chp, "%c-axis: %.3f\r\n ", axis_id[i], cooked[i]);
      }


     chprintf(chp, "LSM6DSL Gyroscope cooked data...\r\n");
        for(i = 0; i < LSM6DSL_GYRO_NUMBER_OF_AXES; i++) {
          chprintf(chp, "%c-axis: %.3f - R:%d\r\n", axis_id[i], cooked[i], retval);
        }

     chThdSleepMilliseconds(500);
   }
}



