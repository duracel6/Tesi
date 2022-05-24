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

#include "shell.h"
#include "chprintf.h"
#include "thdgps.h"

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

/* Can be measured using dd if=/dev/xxxx of=/dev/null bs=512 count=10000.*/
/*static void cmd_temp(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) argc;
  (void) argv;
  chprintf(chp, "\r\n\nstopped\r\n");
}*/

/*static const ShellCommand commands[] = {
  {"temp", cmd_temp},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD2,
  commands
}*/

static THD_WORKING_AREA(waBlinker, 256);
static THD_FUNCTION(thdBlinker, arg){
  (void) arg;
  chRegSetThreadName("Blinker");
  while(TRUE){
    chThdSleepMilliseconds(50);
  }
}

static THD_WORKING_AREA(waGps, 512);
/*
 * Application entry point.
 */
int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Shell manager initialization.
   */
  //shellInit();



  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO + 1, thdBlinker, NULL);
  chThdCreateStatic(waGps, sizeof(waGps), NORMALPRIO + 1, thdGps, NULL);




  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    /*thread_t *tpShell = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                            "shell", NORMALPRIO + 1,
                                            shellThread, (void *)&shell_cfg1);
    chThdWait(tpShell);
                Waiting termination.             */

    chThdSleepMilliseconds(200);
  }
}
