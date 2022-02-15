/* LICENSE=GPL */

/* caliper uinput driver
** written by vordah@gmail.com
*/

/* compile with libusb, use gcc -lusb -lpthread */

#include <stdlib.h>
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

#include "caliper.h"
#include "threads.h"

pthread_mutex_t beep_mutex = PTHREAD_MUTEX_INITIALIZER;

#define TIMEOUT 3000
#define WAITINBETWEEN 500000

/* From HID specification */
#define SET_REQUEST 0x21
#define SET_REPORT 0x09 
#define OUTPUT_REPORT_TYPE 0x02

int pcspeaker_register(struct caliper *device)
{
  char name[256];
  device->have_speaker = 0;
  device->have_softbeep = 0;
  if(device->speakername == NULL)
  {
    perror("continuing with no pc speaker");
    return -1;
  }
  if(device->speakername[0] == '/')
  {
    /* if speakername starts with / it is tried to
       be opened as PC speaker (beep) */
    device->spfd = open(device->speakername, O_RDWR);
    if(device->spfd < 0)
    {
      perror("open pc speaker device");
      return -1;
    }
    if(ioctl(device->spfd, EVIOCGNAME(sizeof(name)), name) < 0) {
      perror("evdev ioctl");
    }
    else
    {
      if(verbose)
        printf("Using \"%s\" to beep.\n", name);
    }
    device->have_speaker = 1;
  }
  if(device->speakername[0] != '/')
  {
     /* if speakername doesn't start with / it is tried to
        be used as a shell command to generate beep */
    {
      if(verbose)
        printf("Using system command \"%s\" as softbeep.\n", device->speakername);
    }
    device->have_softbeep = 1;
  }
  return 0;
}

/* unregister the device from uinput system
*/
int pcspeaker_unregister(struct caliper *device)
{
  /* uinput de-initialization */
  if(device->have_speaker)
    close(device->spfd);

  return 0;
}

void set_beep(struct caliper *device, int freq)
{
  struct input_event beep;

  if(device->have_speaker)
  {
    beep.type = EV_SND;
    beep.code = SND_TONE;
    beep.value = freq;
    
    write(device->spfd, &(beep), sizeof(beep));
  }
}

void *beep_handler(void *data)
{
  struct caliper *device = (struct caliper *) data;
  int i;
  int freq1 = 2000, freq2 = 2400;

  if(device->have_speaker)
  {
    switch(device->beep_type)
    {
    case 1:
      set_beep(device, freq1);
      usleep(100000);
      set_beep(device, 0);
      break;
    case 2:
      for(i = 0; i < 2; i++)
      {
        set_beep(device, freq2);
        usleep(100000);
        set_beep(device, 0);
        usleep(50000);
      }
      set_beep(device, freq2);
      usleep(100000);
      set_beep(device, 0);
      break;
    }
  }
//  pthread_exit(0);
  return NULL;
}

void beep(struct caliper *device, int type)
{
  if(device->have_speaker)
  {
    device->beep_type = type;
    if(rc[THREAD_BEEP] == 0)
      pthread_join( thread[THREAD_BEEP], NULL);
    rc[THREAD_BEEP] = pthread_create( &thread[THREAD_BEEP], NULL, &beep_handler, (void *) (device) );
  }
  if(device->have_softbeep)
  {
    // system("beep &");
    // for systems without internal PC speaker
    // system("aplay -q /usr/share/sounds/sound-icons/beginning-of-line &");
    system(device->speakername);
  }
}
