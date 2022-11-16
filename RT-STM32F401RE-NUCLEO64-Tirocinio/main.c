/*
 PLAY Embedded demos - Copyright (C) 2014...2019 Rocco Marco Guglielmi

 This file is part of PLAY Embedded demos.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "ch.h"
#include "hal.h"
#include "thdgps.h"
#include "thdmems.h"
#include "chprintf.h"
#include "chobjfifos.h"
#include "thdSD.h"
/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/
#define MAX_AXIS_NUMBER     3U
#define GNSS                3U
#define MAX_LENGHT_BUFF 40

static char *GNSS_id[3] = {"LATITUDE", "LONGITUDE", "SPEED"};


typedef struct fifo_data_s{
  int type;
  float data[3];

}fifo_data;




BaseSequentialStream * chp = (BaseSequentialStream*)&SD2;
static THD_WORKING_AREA(waBlinker, 256);

 static THD_FUNCTION(thdBlinker, arg){
 (void) arg;
 chRegSetThreadName("Blinker");
 while(TRUE){
 chThdSleepMilliseconds(50);
 }
}

static THD_WORKING_AREA(waGps, 1024);
static THD_WORKING_AREA(waMems, 1024);
static THD_WORKING_AREA(waSD, 1024);

static objects_fifo_t fifo;
static fifo_data objbuff[MAX_LENGHT_BUFF];
static msg_t msgBuff[MAX_LENGHT_BUFF];



int main(void) {
  /*
   *
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  sdStart(&SD2, NULL);

  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO - 1, thdBlinker, NULL);
  chThdCreateStatic(waGps, sizeof(waGps), NORMALPRIO + 1, thdGps, &fifo);
  chThdCreateStatic(waMems, sizeof(waMems), NORMALPRIO + 1, thdMems, &fifo);
  chThdCreateStatic(waSD, sizeof(waSD), NORMALPRIO + 1, thdSD, &fifo);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  chFifoObjectInit(&fifo, sizeof(fifo_data), MAX_LENGHT_BUFF, objbuff, msgBuff);
  uint32_t i;

  while (true) {
    msg_t msg;
    fifo_data *objp=(fifo_data*)NULL;
    objp->type=0;
    msg = chFifoReceiveObjectTimeout(&fifo, (void**)&objp, TIME_MS2I(500));
    if(msg==0 && objp!=NULL){
    switch(objp->type){

    case 1:
      for(i = 0; i <GNSS; i++) {
                 chprintf(chp, "%s: %.3f\r\n", GNSS_id[i], objp->data[i]);
               }
               break;
    case 2:
            chprintf(chp, "%.3f,%.3f,%.3f\r\n",objp->data[0],objp->data[1],objp->data[2]);
          break;

    default:
      break;
    }
    chFifoReturnObject(&fifo, (void*)objp);
    }
    chThdSleepMilliseconds(10);
  }
}


