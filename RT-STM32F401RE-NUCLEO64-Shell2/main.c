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
#include <string.h>

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
#define INDENT_SPACES "  "

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

/*static THD_WORKING_AREA(waBlinker, 256);
 static THD_FUNCTION(thdBlinker, arg){
 (void) arg;
 chRegSetThreadName("Blinker");
 while(TRUE){
 chThdSleepMilliseconds(50);
 }*/
//}

static THD_WORKING_AREA(waGps, 1024);
/*
 * Application entry point.
 */
/*
bool gpsgetline2(char *line) {
  char c = '\0';

  char * p = line;

  while( ( c = sdGet(&SD1) )  != '$' );
  *p++ = (char)c;

  p += sdReadTimeout(&SD1, p, 79, 50);
  *p++ = '\0';

  return true;
}
*/
static char line[100];
static thread_reference_t trp;

void rxchar(UARTDriver *uartp, uint16_t c) {

  (void)uartp;
  char ch = (char)c;
  static int k = 0;

  if( k == 0 ) {
    if( ch == '$' ){
      line[k++] = ch;
    }
  } else {
    line[k++] = ch;
    if( ch == '\n' ) {
      line[k++] = '\0';
      if( trp != NULL )
      chThdResumeI( &trp, 0 );
      k=0;
    }
  }
}

UARTConfig uart_cfg_1 = {
  NULL,
  NULL,
  NULL,
  rxchar,
  NULL,
  NULL,
  9600,
  0,
  USART_CR2_STOP1_BITS,
  0
};

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


  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));

  //sdStart(&SD1, &sd1_cfg);
  uartStart(&UARTD1, &uart_cfg_1);

  palSetPadMode(GPIOA, 8, PAL_MODE_OUTPUT_PUSHPULL);
  sdStart(&SD2, NULL);

  // RESET SEQUENCE
  palClearPad(GPIOA, 8);
  chThdSleepMilliseconds(100);
  palSetPad(GPIOA, 8);
  chThdSleepMilliseconds(1000);
  palClearPad(GPIOA, 8);
  chThdSleepMilliseconds(2000);
  palSetPad(GPIOA, 8);

  /*
   * Shell manager initialization.
   */
  //shellInit();

  //chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO + 1, thdBlinker, NULL);
  //chThdCreateStatic(waGps, sizeof(waGps), HIGHPRIO, thdGps, NULL);

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
    chThdSuspendS(&trp);
    chprintf( (BaseSequentialStream*) &SD2, "RECEIVED: \"%s\"\n\r", line);
    chThdSleepMilliseconds(20);


  }
}


