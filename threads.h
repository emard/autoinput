#ifndef _CALIPER_THREADS_H
#define _CALIPER_THREADS_H

/* LICENSE=GPL */

/* caliper uinput driver
** written by vordah@gmail.com
*/

/* compile with libusb, use gcc -lusb -lpthread */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <asm/types.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <malloc.h>
// #include <hid.h>
#include <termios.h> /* POSIX terminal control definitions */
//#include <lockdev.h> /* serial line locking */
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#if 0
#include "cmdline.h"
#include "input_keynames.h"
#include "caliper.h"
#include "keypress.h"
#include "beep.h"
#endif

extern pthread_mutex_t mutex1;
#if 0
extern pthread_mutex_t conn_mutex;
#endif

typedef enum {
  THREAD_INPUT = 0,
  THREAD_BEEP,
  THREAD_KEYPRESS,
  THREAD_MAX
} thread_index;

extern int rc[THREAD_MAX];
extern pthread_t thread[THREAD_MAX];

void terminate(int signal);

#endif
