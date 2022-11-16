#include "ch.h"
#include "hal.h"

#include "Fusion.h"
#include "chprintf.h"
#include "lsm6dsl.h"
#include "chobjfifos.h"
#include "lsm303agr.h"
#include "MadgwickAHRS.h"
#include "quaternion.h"

// SCL_PIN     GPIOB, 8
// SDA_PIN     GPIOB, 9



#define MAX_AXIS_NUMBER     3U
systimestamp_t previusTimestamp=0;

typedef struct fifo_data_s{
  int type;
  float data[3];

} fifo_data;



uint32_t i;

/* Array per immagazzinare i dati  */
  static float cookedGyro[MAX_AXIS_NUMBER];
  static float cookedMag[MAX_AXIS_NUMBER];
  static float cookedAcc [MAX_AXIS_NUMBER];
  //static float phi,theta,psi;
  //static float gyrox,gyroy,gyroz;
  //static float accx,accy,accz;
  //static float magx,magy,magz;
  //static int rawGyro[MAX_AXIS_NUMBER];
  //static int rawMag[MAX_AXIS_NUMBER];
  //static int rawAcc [MAX_AXIS_NUMBER];
  FusionVector gyroscope, accelerometer,magnetometer;
  FusionAhrs ahrs;
  FusionEuler euler;
  FusionQuaternion quaternion;
  static float clkPerSecond=TIME_US2I(9615);

/* Configurazione standard per i mems */
  static const I2CConfig i2ccfg = {OPMODE_I2C, 400000, FAST_DUTY_CYCLE_2};


/* definizione del puntatore al'interfaccia seriale */

/* Configurazione del driver per l'imu iNEMO */

  static LSM6DSLDriver LSM6DSLD1;

  static const LSM6DSLConfig lsm6dslcfg = { &I2CD1, &i2ccfg, LSM6DSL_SAD_VCC, NULL,
                                          NULL, LSM6DSL_ACC_FS_4G,
                                          LSM6DSL_ACC_ODR_104Hz, NULL, NULL, LSM6DSL_GYRO_FS_250DPS,
                                          LSM6DSL_GYRO_ODR_104Hz};

  static LSM303AGRDriver LSM303AGRD1;

  static const LSM303AGRConfig lsm303agrcfg = {&I2CD1,&i2ccfg,NULL,NULL,
                                               LSM303AGR_ACC_FS_4G,LSM303AGR_ACC_ODR_100Hz,
                                               NULL,NULL,LSM303AGR_COMP_ODR_50HZ,};
  /* Questo dato va modificato in base alla frequesnza di campionamento scelta per i vari moduli*/


/*dichiarazione thread MEMS*/
 THD_FUNCTION(thdMems, arg) {


   objects_fifo_t* ofp=(objects_fifo_t*) arg;
   chRegSetThreadName("MEMS Thread");




   /* configurazione pin per l'i2c */
   palSetLineMode( LINE_ARD_D15,
                   PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN
   );

   palSetLineMode(LINE_ARD_D14,
                  PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN
   );

   /* inizializzazione driver imu */
   lsm6dslObjectInit(&LSM6DSLD1);

   /* Attivazione driver imu */
   lsm6dslStart(&LSM6DSLD1, &lsm6dslcfg);

   /* inizializzazione driver magnetmetro */
   lsm303agrObjectInit(&LSM303AGRD1);

   /* Attivazione driver magnetometro */
   lsm303agrStart(&LSM303AGRD1, &lsm303agrcfg);


   while (true){
     systimestamp_t timestamp=chVTGetTimeStamp();
    fifo_data* objp = NULL;

    objp = (fifo_data*)chFifoTakeObjectTimeout(ofp, TIME_I2MS(100));
    if(objp !=NULL){

       lsm6dslAccelerometerReadCooked(&LSM6DSLD1, cookedAcc);
       //lsm6dslAccelerometerReadRaw(&LSM6DSLD1, rawAcc);


       lsm6dslGyroscopeReadCooked(&LSM6DSLD1, cookedGyro);
       //lsm6dslGyroscopeReadRaw(&LSM6DSLD1, rawGyro);


       lsm303agrCompassReadCooked(&LSM303AGRD1, cookedMag);
       //lsm303agrCompassReadCooked(&LSM303AGRD1, rawMag);
       accelerometer.array[0]=cookedAcc[0]/1000;
       accelerometer.array[1]=cookedAcc[1]/1000;
       accelerometer.array[2]=cookedAcc[2]/1000;

       gyroscope.array[0]=cookedGyro[0];
       gyroscope.array[1]=cookedGyro[1];
       gyroscope.array[2]=cookedGyro[2];

       magnetometer.array[0] =cookedMag[0];
       magnetometer.array[1] =cookedMag[1];
       magnetometer.array[2] =cookedMag[2];
       //MadgwickAHRSupdate(gyrox,gyroy,gyroz,accx,accy,accz,magx,magy,magz);
       //MadgwickAHRSupdateIMU(gyrox, gyroy, gyroz, accx, accy, accz);
       FusionAhrsReset(&ahrs);
       FusionAhrsInitialise(&ahrs);


       const float deltaTime=(timestamp-previusTimestamp)/clkPerSecond ;
       previusTimestamp=timestamp;

       FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, deltaTime);
       euler=FusionQuaternionToEuler(ahrs.quaternion);


       //quatern2euler(q0, q1, q2, q3, &phi, &theta, &psi);

       objp->type=2;
       objp->data[0]=euler.angle.pitch;
       objp->data[1]=euler.angle.roll;
       objp->data[2]=euler.angle.yaw;
       chFifoSendObject(ofp, objp);
       chThdSleepMilliseconds(100);
    }
   }
}



