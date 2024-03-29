#include "ch.h"
#include "hal.h"
#include "minmea.h"
#include "chprintf.h"
#include "chscanf.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INDENT_SPACES "  "

static char line[MINMEA_MAX_LENGTH];
static thread_reference_t trp;

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

  (void)arg;
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
    int i;

    trp = (thread_reference_t) NULL;
    uartStart(&UARTD1, &uart1_cfg);
    chThdSuspendS(&trp);

   trp = (thread_reference_t) NULL;
    uartStop(&UARTD1);

    chprintf(chp2, "RECEIVED:  \"%s\"\n\r\r", line);
    switch (minmea_sentence_id(line, true)) {
      case MINMEA_SENTENCE_RMC: {
        struct minmea_sentence_rmc frame;
        if (minmea_parse_rmc(&frame, line)) {
          chprintf(
              chp2,"$xxRMC: raw coordinates and speed: (%f,%f) %d/%d\n\r",
              conversion(frame.latitude.value), conversion(frame.longitude.value),
               frame.speed.value, frame.speed.scale);
          chprintf(
              chp2,
              INDENT_SPACES "$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n\r",
              minmea_rescale(&frame.latitude, 1000),
              minmea_rescale(&frame.longitude, 1000),
              minmea_rescale(&frame.speed, 1000));
          chprintf(
              chp2,
              INDENT_SPACES "$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n\r",
              minmea_tocoord(&frame.latitude), minmea_tocoord(&frame.longitude),
              minmea_tofloat(&frame.speed)*1.852);
        }
        else {
          chprintf(chp2, INDENT_SPACES "$xxRMC sentence is not parsed\n\r");
        }
      }
        break;

      case MINMEA_SENTENCE_GGA: {
        struct minmea_sentence_gga frame;
        if (minmea_parse_gga(&frame, line)) {
                   chprintf(chp2, INDENT_SPACES "$xxGGA: coordinates (%f,%f), altitude=%f, height=%f \n\r",
                   conversion(frame.latitude.value),conversion(frame.longitude.value),frame.altitude,frame.height);
        }
        else {
          chprintf(chp2, INDENT_SPACES "$xxGGA sentence is not parsed\n\r");
        }
      }
        break;

     case MINMEA_SENTENCE_GST: {
        struct minmea_sentence_gst frame;
        if (minmea_parse_gst(&frame, line)) {
          chprintf(
              chp2,
              INDENT_SPACES "$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)\n\r",
              frame.latitude_error_deviation.value,
              frame.latitude_error_deviation.scale,
              frame.longitude_error_deviation.value,
              frame.longitude_error_deviation.scale,
              frame.altitude_error_deviation.value,
              frame.altitude_error_deviation.scale);
          chprintf(
              chp2,
              INDENT_SPACES "$xxGST fixed point latitude,longitude and altitude error deviation"
              " scaled to one decimal place: (%d,%d,%d)\n\r",
              minmea_rescale(&frame.latitude_error_deviation, 10),
              minmea_rescale(&frame.longitude_error_deviation, 10),
              minmea_rescale(&frame.altitude_error_deviation, 10));
          chprintf(
              chp2,
              INDENT_SPACES "$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)\n\r",
              minmea_tofloat(&frame.latitude_error_deviation),
              minmea_tofloat(&frame.longitude_error_deviation),
              minmea_tofloat(&frame.altitude_error_deviation));
        }
        else {
          chprintf(chp2, INDENT_SPACES "$xxGST sentence is not parsed\n\r");
        }
      }
        break;

      case MINMEA_SENTENCE_GSV: {
        struct minmea_sentence_gsv frame;
        if (minmea_parse_gsv(&frame, line)) {
          chprintf(chp2, INDENT_SPACES "$xxGSV: message %d of %d\n\r", frame.msg_nr,
                   frame.total_msgs);
          chprintf(chp2, INDENT_SPACES "$xxGSV: sattelites in view: %d\n\r",
                   frame.total_sats);
          for (i = 0; i < 4; i++)
            chprintf(
                chp2,
                INDENT_SPACES "$xxGSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n\r",
                frame.sats[i].nr, frame.sats[i].elevation, frame.sats[i].azimuth,
                frame.sats[i].snr);
        }
        else {
          chprintf(chp2, INDENT_SPACES "$xxGSV sentence is not parsed\n\r");
        }
      }
        break;

      case MINMEA_SENTENCE_VTG: {
        struct minmea_sentence_vtg frame;
        if (minmea_parse_vtg(&frame, line)) {
          chprintf(chp2, INDENT_SPACES "$xxVTG: true track degrees = %f\n\r",
                   minmea_tofloat(&frame.true_track_degrees));
          chprintf(chp2, INDENT_SPACES "        magnetic track degrees = %f\n\r",
                   minmea_tofloat(&frame.magnetic_track_degrees));
          chprintf(chp2, INDENT_SPACES "        speed knots = %f\n\r",
                   minmea_tofloat(&frame.speed_knots));
          chprintf(chp2, INDENT_SPACES "        speed kph = %f\n\r",
                   minmea_tofloat(&frame.speed_kph));
        }
        else {
          chprintf(chp2, INDENT_SPACES "$xxVTG sentence is not parsed\n\r");
        }
      }
        break;

      case MINMEA_SENTENCE_ZDA: {
        struct minmea_sentence_zda frame;
        if (minmea_parse_zda(&frame, line)) {
          chprintf(chp2,
          INDENT_SPACES "$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n\r",
                   frame.time.hours, frame.time.minutes, frame.time.seconds,
                   frame.date.day, frame.date.month, frame.date.year,
                   frame.hour_offset, frame.minute_offset);
        }
        else {
          chprintf(chp2, INDENT_SPACES "$xxZDA sentence is not parsed\n\r");
        }
      }
        break;
      case MINMEA_SENTENCE_GLL: {
        struct minmea_sentence_gll frame;
        if (minmea_parse_gll(&frame, line)) {
          chprintf(
              chp2,
              INDENT_SPACES "$xxGll: raw latitude,longitude and altitude: (%f,%f)\n\r",
              conversion(frame.latitude.value), conversion(frame.longitude.value)
              );
        }

        else {
          chprintf(chp2, INDENT_SPACES "$xxGLL sentence is not parsed\n\r");
        }
      }
        break;

      case MINMEA_INVALID: {
        chprintf(chp2, INDENT_SPACES "$xxxxx sentence is not valid\n\r");
      }
        break;

      default: {
      }
    }
    chThdSleepMilliseconds(2);
  }
}

