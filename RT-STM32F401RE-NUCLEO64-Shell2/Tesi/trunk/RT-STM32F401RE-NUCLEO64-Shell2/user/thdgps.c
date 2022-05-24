#include "ch.h"
#include "hal.h"
#include "thdgps.h"
#include "minmea.h"
#include "chprintf.h"
#include "chscanf.h"
#include <string.h>
#include <ctype.h>

int i;
#define INDENT_SPACES "  "
BaseSequentialStream* chp2 = (BaseSequentialStream*)&SD2;


bool gpsgetline(BaseSequentialStream *chp, char *line, unsigned size ) {
  char *p = line;


  while (true) {
    char c;

    if (streamRead(chp, (uint8_t *)&c, 1) == 0)
      return true;

    if ((c == 8) || (c == 127)) {
      if (p != line) {
        streamPut(chp, 0x08);
        streamPut(chp, 0x20);
        streamPut(chp, 0x08);
        p--;
      }
      continue;
    }
    if (c == '\r') {
      //chprintf(chp, "\n\r");
      *p = 0;
      return false;
    }
    if (c < 0x20)
      continue;
    if (p < line + size - 1) {
      //streamPut(chp, c);
      *p++ = (char)c;
    }
  }
}


static const SerialConfig sd1_cfg =
{
  9600,
  0,
  USART_CR2_STOP1_BITS,
  0
};

THD_FUNCTION(thdGps, arg) {
  (void)arg;
  chRegSetThreadName("Reset");

  palSetPadMode( GPIOA, 10, PAL_MODE_ALTERNATE(7));

  sdStart(&SD1, &sd1_cfg);
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

  char line[MINMEA_MAX_LENGTH];

  while (TRUE) {
    gpsgetline( (BaseSequentialStream*)&SD1, line, MINMEA_MAX_LENGTH);
    chprintf( chp2, "RECEIVED: \"%s\"\n\r", line);
    chThdSleepMilliseconds(20);
    continue;

    switch (minmea_sentence_id(line, true)) {
    case MINMEA_SENTENCE_RMC: {
      struct minmea_sentence_rmc frame;
      if (minmea_parse_rmc(&frame, line)) {
        chprintf(chp2,
            INDENT_SPACES "$xxRMC: raw coordinates and speed: (%ld/%ld,%ld/%ld) %ld/%ld\n",
            frame.latitude.value, frame.latitude.scale, frame.longitude.value,
            frame.longitude.scale, frame.speed.value, frame.speed.scale);
        chprintf(chp2,
            INDENT_SPACES "$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%ld,%ld) %ld\n",
            minmea_rescale(&frame.latitude, 1000),
            minmea_rescale(&frame.longitude, 1000),
            minmea_rescale(&frame.speed, 1000));
        chprintf(chp2,
            INDENT_SPACES "$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
            minmea_tocoord(&frame.latitude), minmea_tocoord(&frame.longitude),
            minmea_tofloat(&frame.speed));
      }
      else {
        chprintf(chp2,INDENT_SPACES "$xxRMC sentence is not parsed\n");
      }
    }
      break;

    case MINMEA_SENTENCE_GGA: {
      struct minmea_sentence_gga frame;
      if (minmea_parse_gga(&frame, line)) {
        chprintf(chp2,INDENT_SPACES "$xxGGA: fix quality: %d\n", frame.fix_quality);
      }
      else {
        chprintf(chp2,INDENT_SPACES "$xxGGA sentence is not parsed\n");
      }
    }
      break;

    case MINMEA_SENTENCE_GST: {
      struct minmea_sentence_gst frame;
      if (minmea_parse_gst(&frame, line)) {
        chprintf(chp2,
            INDENT_SPACES "$xxGST: raw latitude,longitude and altitude error deviation: (%ld/%ld,%ld/%ld,%ld/%ld)\n",
            frame.latitude_error_deviation.value,
            frame.latitude_error_deviation.scale,
            frame.longitude_error_deviation.value,
            frame.longitude_error_deviation.scale,
            frame.altitude_error_deviation.value,
            frame.altitude_error_deviation.scale);
        chprintf(chp2,
            INDENT_SPACES "$xxGST fixed point latitude,longitude and altitude error deviation"
            " scaled to one decimal place: (%ld,%ld,%ld)\n",
            minmea_rescale(&frame.latitude_error_deviation, 10),
            minmea_rescale(&frame.longitude_error_deviation, 10),
            minmea_rescale(&frame.altitude_error_deviation, 10));
        chprintf(chp2,
            INDENT_SPACES "$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
            minmea_tofloat(&frame.latitude_error_deviation),
            minmea_tofloat(&frame.longitude_error_deviation),
            minmea_tofloat(&frame.altitude_error_deviation));
      }
      else {
        chprintf(chp2,INDENT_SPACES "$xxGST sentence is not parsed\n");
      }
    }
      break;

    case MINMEA_SENTENCE_GSV: {
      struct minmea_sentence_gsv frame;
      if (minmea_parse_gsv(&frame, line)) {
        chprintf(chp2,INDENT_SPACES "$xxGSV: message %d of %d\n", frame.msg_nr,
               frame.total_msgs);
        chprintf(chp2,INDENT_SPACES "$xxGSV: sattelites in view: %d\n",
               frame.total_sats);
        for (i = 0; i < 4; i++)
          chprintf(chp2,
              INDENT_SPACES "$xxGSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
              frame.sats[i].nr, frame.sats[i].elevation, frame.sats[i].azimuth,
              frame.sats[i].snr);
      }
      else {
        chprintf(chp2,INDENT_SPACES "$xxGSV sentence is not parsed\n");
      }
    }
      break;

    case MINMEA_SENTENCE_VTG: {
      struct minmea_sentence_vtg frame;
      if (minmea_parse_vtg(&frame, line)) {
        chprintf(chp2,INDENT_SPACES "$xxVTG: true track degrees = %f\n",
               minmea_tofloat(&frame.true_track_degrees));
        chprintf(chp2,INDENT_SPACES "        magnetic track degrees = %f\n",
               minmea_tofloat(&frame.magnetic_track_degrees));
        chprintf(chp2,INDENT_SPACES "        speed knots = %f\n",
               minmea_tofloat(&frame.speed_knots));
        chprintf(chp2,INDENT_SPACES "        speed kph = %f\n",
               minmea_tofloat(&frame.speed_kph));
      }
      else {
        chprintf(chp2,INDENT_SPACES "$xxVTG sentence is not parsed\n");
      }
    }
      break;

    case MINMEA_SENTENCE_ZDA: {
      struct minmea_sentence_zda frame;
      if (minmea_parse_zda(&frame, line)) {
        chprintf(chp2,INDENT_SPACES "$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n",
               frame.time.hours, frame.time.minutes, frame.time.seconds,
               frame.date.day, frame.date.month, frame.date.year,
               frame.hour_offset, frame.minute_offset);
      }
      else {
        chprintf(chp2,INDENT_SPACES "$xxZDA sentence is not parsed\n");
      }
    }
      break;

    case MINMEA_INVALID: {
      chprintf(chp2,INDENT_SPACES "$xxxxx sentence is not valid\n");
    }
      break;

    default: {
    }
    }
    chThdSleepMilliseconds(2000);
  }
}
