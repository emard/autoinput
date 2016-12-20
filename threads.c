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
#include <lockdev.h> /* serial line locking */
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "cmdline.h"
#if 0
#include "input_keynames.h"
#endif
#include "caliper.h"
#if 0
#include "keypress.h"
#endif
#include "beep.h"

#include "threads.h"

pthread_mutex_t mutex1 =     PTHREAD_MUTEX_INITIALIZER;
#if 0
pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#define TIMEOUT 3000
#define WAITINBETWEEN 500000

int rc[THREAD_MAX];
pthread_t thread[THREAD_MAX];

void terminate(int signal)
{
  /* stop all input threads */
  /* other threads (beepers, keyers) will finish normally) */
  /* when there are no input threads to spawn them */
  if(verbose)
    printf("cancelling input thread\n");
  if(rc[THREAD_INPUT] == 0)
    pthread_cancel(thread[THREAD_INPUT]);
}

