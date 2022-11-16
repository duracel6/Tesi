 #include "ch.h"
#include "hal.h"
#include "minmea.h"
#include "chprintf.h"
#include "chscanf.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "chobjfifos.h"
#define INDENT_SPACES "  "

static char line[MINMEA_MAX_LENGTH];
static thread_reference_t trp;
typedef struct {
  int type;
  float data[3];

}fifo_data;

void rxchar(UARTDriver *uartp, uint16_t c) {

  (void)uartp;
  char ch = (char)c;
  static int k = 0;

  if (trp == NULL) {
    k = 0;
    return;
  }

  if (k == 0) {
    if (ch == '$') {
      line[k++] = ch;
    }
  } else {
    line[k++] = ch;

    if (ch == '\n') {
      line[k++] = '\0';
      k = 0;

      chThdResumeI(&trp, 0);
    }
  }
}

UARTConfig uart1_cfg = {NULL, NULL, NULL, rxchar, NULL, NULL, 9600, 0, USART_CR2_STOP1_BITS, 0};

BaseSequentialStream *chp2 = (BaseSequentialStream*) &SD2;
THD_FUNCTION(thdGps, arg) {

  objects_fifo_t* ofp=(objects_fifo_t*) arg;

  chRegSetThreadName("GPS Thread");
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
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

  while (TRUE) {
    fifo_data*objp=NULL;

    trp = (thread_reference_t) NULL;
    uartStart(&UARTD1, &uart1_cfg);
    chThdSuspendS(&trp);

   trp = (thread_reference_t) NULL;
    uartStop(&UARTD1);
    int i;

    switch (minmea_sentence_id(line, true)) {
      case MINMEA_SENTENCE_RMC: {
        objp = (fifo_data*)chFifoTakeObjectTimeout(ofp, TIME_I2MS(100));
            objp->type = 1;
            for(i=0;i<3;i++){
              objp->data[i]=0;
            }
        struct minmea_sentence_rmc frame;
        if (minmea_parse_rmc(&frame, line)) {
          objp->data[0] = minmea_tocoord(&frame.latitude);
              objp->data[1] = minmea_tocoord(&frame.longitude);
              objp->data[2] = minmea_tofloat(&frame.speed)*1.852;
              chFifoSendObject(ofp, objp);
        }

      }
        break;


      default: {
      }
    }
    chThdSleepMilliseconds(150);
  }
}

