#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "chobjfifos.h"
#include "hal_mmc_spi.h"


typedef struct fifo_data_s{
  int type;
  float data[3];

} fifo_data;
#define MAX_AXIS_NUMBER     3U
#define GNSS                3U
#define MAX_LENGHT_BUFF 40
static char *axis_id[MAX_AXIS_NUMBER] = {"Phi", "Theta", "Psi"};
static char *GNSS_id[3] = {"LATITUDE", "LONGITUDE", "SPEED"};
static objects_fifo_t fifo;
static fifo_data objbuff[MAX_LENGHT_BUFF];
static msg_t msgBuff[MAX_LENGHT_BUFF];
static uint32_t i;
static MMCDriver MCCDriver;
BaseSequentialStream * chp1 = (BaseSequentialStream*)&SD2;
static SPIConfig spiConfig={
     false,


};


static MMCConfig config={

}
THD_FUNCTION(thdSD, arg){
  objects_fifo_t* ofp=(objects_fifo_t*) arg;

  /*configurazione pin per SPI*/
  palSetPadMode(GPIOA,5,PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOA,6,PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOA,7,PAL_MODE_ALTERNATE(5));
  palSetPadMode(GPIOA,6,PAL_MODE_ALTERNATE(5));
  palSetPad(GPIOB,6);
   chRegSetThreadName("SD Thread");

   mmcObjectInit(&MCCDriver);
   mmcStart(mmcp, config)



   sdStart(&SD2, NULL);
   chFifoObjectInit(&fifo, sizeof(fifo_data), MAX_LENGHT_BUFF, objbuff, msgBuff);
   while (TRUE)

   {
     msg_t msg;
        fifo_data *objp=(fifo_data*)NULL;
        objp->type=0;


        msg = chFifoReceiveObjectTimeout(&fifo, (void**)&objp, TIME_MS2I(500));
        if(msg==0 && objp!=NULL){
        switch(objp->type){

        case 1:
          for(i = 0; i <GNSS; i++) {
                     //chprintf(chp, "%s: %.3f\r\n", GNSS_id[i], objp->data[i]);
                   }
                   break;
        case 2:
          for(i = 0; i <GNSS; i++) {
                //chprintf(chp, "%s: %.3f\r\n",axis_id[MAX_AXIS_NUMBER], objp->data[i]);
        }
              break;

        default:
          break;
        }
     chThdSleepMilliseconds(100);

   }
}
}
