#ifndef CALIPER_CALIPER_H
#define CALIPER_CALIPER_H

/* LICENSE=GPL */

#include <linux/input.h>
#include <linux/uinput.h>
#include <termios.h>

/* maximum length of value string in struct caliper */
#define VALUE_MAX 20

typedef enum {
  STATE_IDLE = 0,
  STATE_IDLE60S,
  STATE_1OFF,
  STATE_2OFF,
  STATE_ADDR,
  STATE_SENDING,
  STATE_STABLE,
  STATE_B4SIGN,
  STATE_ZERO,
  STATE_VALUE
} device_state;

typedef enum {
  PROTOCOL_MITUTOYO = 0,
  PROTOCOL_DENVER,
  PROTOCOL_METTLERTOLEDO,
  PROTOCOL_SIMPLE
} device_protocol;

typedef enum {
  PARITY_NONE = 0,
  PARITY_ODD,
  PARITY_EVEN
} device_parity;

struct keysequence {
  struct keysequence *next; /* NULL terminated */
  int key;   /* key number e.g. KEY_ENTER */ 
  int press; /* press down (1) or release (0) */
};

struct caliper {
  int uifd, hwfd; /* os-side uinput fd (read/write) and origin (readonly) */
  struct uinput_user_dev device;
  char *name; /* originating device name e.g. /dev/ttyS0 */
  device_protocol protocol; /* protocol ID (mitutoy, denver, etc.) */
  device_state state; /* originating device state machine */
  speed_t bps; /* baud rate */
  device_parity parity; /* 0-none, 1-odd, 2-even */
  char value[VALUE_MAX]; /* value gets copied here (for the on-one correction) */
  char sendvalue[VALUE_MAX];
  int value_ptr;
  int suppress_plus;
  int suppress_zero, cancel_zero;
  int debounce; /* us delay to reject too fast incoming data */
  int output_delay; /* us to delay beween outputting keypresses */
  /* beep */
  char *speakername; /* PC Speaker device e.g. /dev/input/event1 */
  int spfd; /* PC speaker fd */
  int have_speaker, beep_type;
  int have_softbeep;
  int persist; /* persistently try to connect */
  int connected;
  struct keysequence *ascii2key[256];
  struct caliper *next;
};

extern int verbose;

void *input_handler_serial(void *data);
void *input_handler_udp(void *data);

#endif
