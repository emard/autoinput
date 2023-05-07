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

#include "cmdline.h"
#include "input_keynames.h"
#include "caliper.h"
#include "keypress.h"
#include "beep.h"

pthread_mutex_t addchar_mutex = PTHREAD_MUTEX_INITIALIZER;

const char *key2string(int key)
{
  int i;
  int n = sizeof(key_name)/sizeof(key_name[0]);

  for(i = 0; i < n; i++)
  {
    if(key == key_name[i].key)
      return key_name[i].name;
  }
  return "*UNKNOWN*";
}

int string2key(char *name)
{
  int i;
  int n = sizeof(key_name)/sizeof(key_name[0]);

  if(name == NULL)
    return -1;

  for(i = 0; i < n; i++)
  {
    if(strcasecmp(name, key_name[i].name) == 0)
      return key_name[i].key;
  }
  return -1;
}

void clearsequence(struct caliper *device, char origin)
{
  struct keysequence *ptr, *next, *sequence = device->ascii2key[(int) origin];

  for(ptr = sequence; ptr != NULL; ptr = next)
  {
    next = ptr->next;
    free(ptr);
  }
  device->ascii2key[(int) origin] = NULL;
}

void addsequence(struct caliper *device, char origin, int key, int press)
{
  struct keysequence **sequence, **ptr /*, **prev */;
  
  sequence = &(device->ascii2key[(int) origin]);
  
  if(sequence == NULL)
    return;

  /* prev = NULL; */
  for(ptr = sequence; (*ptr) != NULL; ptr = &((*ptr)->next))
    /* prev = ptr */ ;

  if(ptr)
  {
    *ptr = (struct keysequence *) malloc(sizeof(struct keysequence));
    (*ptr)->next = NULL;
    (*ptr)->key = key;
    (*ptr)->press = press;
  }
}

void pressrelease(struct caliper *device, char origin, int key)
{
  addsequence(device, origin, key, 1);
  addsequence(device, origin, key, 0);
}

void addkeypress(struct caliper *device, char origin, char *key)
{
  int press = -1;
  char *name;

  if(key == NULL)
    return;

  name = key;
  if(*key == '+')
  {
    press = 1;
    name++;
  }
  if(*key == '-')
  {
    press = 0;
    name++;
  }
  if(press < 0)
    pressrelease(device, origin, string2key(name));
  else
    addsequence(device, origin, string2key(name), press);
}

void addkeysequence(struct caliper *device, char origin, char *string)
{
  char *key, *saveptr, *sequence;

  sequence = (char *) malloc(strlen(string)+1);
  strcpy(sequence, string);

  for(key = strtok_r(sequence, ",", &saveptr); 
      key != NULL; 
      key = strtok_r(NULL, ",", &saveptr))
    addkeypress(device, origin, key);
  
  free(sequence);
}

void setkeysequence(struct caliper *device, char origin, char *string)
{
  clearsequence(device, origin);
  addkeysequence(device, origin, string);
}

void printkey(struct keysequence *ptr)
{
  printf("%c%s", (ptr->press ? '+' : '-'), key2string(ptr->key));
}

void printsequence(struct caliper *device, char i)
{
  struct keysequence *ptr;
  char d = ' ';
  
  printf("'%c':", (char) i);
  for(ptr = device->ascii2key[(int) i]; ptr != NULL; ptr = ptr->next, d = ',')
  {
    printf("%c", d);
    printkey(ptr);
  }
  printf("\n");
}

void printmapping(struct caliper *device)
{
  int i;

  printf("Serial char to key mapping:\n");
  for(i = 0; i < 256; i++)
  {
    if(device->ascii2key[i])
      printsequence(device, i);
  }
}

int clearchar(struct caliper *device)
{
  pthread_mutex_lock(&addchar_mutex);
  device->value_ptr = 0;
  device->value[0] = '\0';
  pthread_mutex_unlock(&addchar_mutex);
  return 0;
}

int addchar(struct caliper *device, char input)
{
  int retval = 0;
  int n = sizeof(device->value)/sizeof(device->value[0]);

  /* mutex this */
  pthread_mutex_lock(&addchar_mutex);
  if(device->value_ptr < n)
  {
    device->value[device->value_ptr++] = input;
    retval = 0;
  }
  else
  {
    device->value[n-1] = '\0';
    retval =-1;
  }
  pthread_mutex_unlock(&addchar_mutex);

  return retval;
}

/*
** note struct *device can be used for some state
** variables in case we'll need them. currently it's
** unused
** this is state machine for mitutoyo caliper
** should suppress reporting values 911 and 912
** 911 happens when caliper is off
** 912 happens sometimes on some caliper (150 mm)
*/
int key_convert_mitutoyo(struct caliper *device, char input, struct input_event *event)
{
  int output = 0, complete = 0;
  event->type = EV_KEY;
  event->value = 1;
  //printf("state=%d\n", device->state);
  switch(device->state)
  {
  case STATE_IDLE:
  case STATE_IDLE60S:
    switch(input)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      // system("/usr/bin/beep -f 2000 -l 100 &");
      device->state = STATE_ADDR;
      break;
    case '9':
      device->state = STATE_1OFF;
      break;
    }
    break;
  case STATE_1OFF:
    switch(input)
    {
    case '1':
      device->state = STATE_2OFF;
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_2OFF:
    switch(input)
    {
    case '1':
      device->state = STATE_IDLE;
      #if 0
      output = 1;
      input = '0';
      // beep(device, 2); /* beep for not accepted value */
      clearchar(device);
      addchar(device, input);
      addchar(device, '\0');
      complete = 1;
      #endif
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_SENDING:
  case STATE_STABLE:
  case STATE_ADDR:
    switch(input)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      device->state = STATE_B4SIGN;
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_B4SIGN:
    switch(input)
    {
    case '-':
      output = 1;
    case '+':
      device->state = STATE_VALUE;
      if(device->suppress_zero)
      {
        device->state = STATE_ZERO;
        device->cancel_zero = 4;
      }
      clearchar(device);
      if(device->suppress_plus <= 0)
        output = 1;
      if(output)
        addchar(device, input);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_ZERO:
    switch(input)
    {
    case '0':
      /* zero suppression */
      if(--device->cancel_zero <= 0)
        device->state = STATE_VALUE;
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    case '\r':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      output = 1;
      complete = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_VALUE:
    switch(input)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      output = 1;
      break;
    case '\r':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      complete = 1;
      output = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  }
  return complete;
}


/*
** note struct *device can be used for some state
** variables in case we'll need them. currently it's
** unused
** this is state machine for BOSCH (Denver Instruments / Sartorious)
** analytical balance
*/
int key_convert_denver(struct caliper *device, char input, struct input_event *event)
{
  int output = 0, complete = 0;
  event->type = EV_KEY;
  event->value = 1;

  switch(device->state)
  {
  case STATE_IDLE:
  case STATE_IDLE60S:
  case STATE_1OFF:
  case STATE_2OFF:
  case STATE_ADDR:
  case STATE_SENDING:
  case STATE_STABLE:
  case STATE_B4SIGN:
    switch(input)
    {
    case '-':
      output = 1;
    case '+':
    case ' ':
      device->state = STATE_VALUE;
      if(device->suppress_zero)
      {
        device->state = STATE_ZERO;
        device->cancel_zero = 4;
      }
      clearchar(device);
      if(device->suppress_plus <= 0)
        output = 1;
      if(output)
        addchar(device, input);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_ZERO:
    switch(input)
    {
    case ' ':
      /* zero suppression */
      if(--device->cancel_zero <= 0)
        device->state = STATE_VALUE;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    case 'g':
    case '\r':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      output = 1;
      complete = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_VALUE:
    switch(input)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      output = 1;
      break;
    case ' ':
    case 'g':
      break;
    case '\r':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      complete = 1;
      output = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  }
  return complete;
}

/*
** note struct *device can be used for some state
** variables in case we'll need them. currently it's
** unused
** this is state machine for BOSCH (Denver Instruments / Sartorious)
** analytical balance
*/
int key_convert_mettlertoledo(struct caliper *device, char input, struct input_event *event)
{
  int output = 0, complete = 0;
  /* this sequence enables sending weight by pressing menu key */
  char send_on_keypress[] = "ST 1\r\n"; 
  // char send_on_keypress[] = "ST 1\r\nS\r\n"; 
  event->type = EV_KEY;
  event->value = 1;
  
  //printf("state=%d\n", device->state);

  switch(device->state)
  {
  case STATE_IDLE:
  case STATE_IDLE60S:
  case STATE_1OFF:
  case STATE_2OFF:
  case STATE_ADDR:
    switch(input)
    {
    case '\0':
      /* null sequence from idle state: re-enable sending weight by keypress */
      write(device->hwfd, send_on_keypress, strlen(send_on_keypress));
      break;
    case 'S':
      clearchar(device);
      device->state = STATE_SENDING;
      break;
    }
    break;
  case STATE_SENDING:
    switch(input)
    {
      case ' ':
      case 'T':
      case 'A':
        break;
      case 'S':
        device->state = STATE_STABLE;
        break;
      case '\0':
      case '\n':
        clearchar(device);
        device->state = STATE_IDLE60S;
        output = 0;
        complete = -1;
        break;
      case '\r':
        break;
    }
    break;
  case STATE_STABLE:
    switch(input)
    {
      case ' ':
        clearchar(device);
        device->state = STATE_B4SIGN;
        break;
      case '\0':
        device->state = STATE_IDLE60S;
        break;
    }
    break;
  case STATE_B4SIGN:
    switch(input)
    {
    case ' ':
      break;
    case '-':
      output = 1;
    case '+':
      device->state = STATE_VALUE;
      if(device->suppress_zero)
      {
        device->state = STATE_ZERO;
        device->cancel_zero = 4;
      }
      clearchar(device);
      if(device->suppress_plus <= 0)
        output = 1;
      if(output)
        addchar(device, input);
      break;
    case '0':
      device->state = STATE_ZERO;
      /* zero suppression */
      if(--device->cancel_zero <= 0)
        device->state = STATE_VALUE;
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    default:
      device->state = STATE_IDLE60S;
      break;
    }
    break;
  case STATE_ZERO:
    switch(input)
    {
    case '0':
      /* zero suppression */
      if(--device->cancel_zero <= 0)
        device->state = STATE_VALUE;
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    case 'g':
    case '\r':
      addchar(device, '\0');
      device->state = STATE_IDLE60S;
      output = 1;
      complete = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE60S;
      break;
    }
    break;
  case STATE_VALUE:
    switch(input)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      output = 1;
      break;
    case ' ':
    case 'g':
      break;
    case '\r':
      addchar(device, '\0');
      device->state = STATE_IDLE60S;
      complete = 1;
      output = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE60S;
      break;
    }
    break;
  }
  return complete;
}

int key_convert_simple(struct caliper *device, char input, struct input_event *event)
{
  int output = 0, complete = 0;
  event->type = EV_KEY;
  event->value = 1;

  switch(device->state)
  {
  case STATE_IDLE:
  case STATE_IDLE60S:
  case STATE_1OFF:
  case STATE_2OFF:
  case STATE_ADDR:
  case STATE_SENDING:
  case STATE_STABLE:
  case STATE_B4SIGN:
    switch(input)
    {
    case '-':
      output = 1;
    case '+':
    case ' ':
      device->state = STATE_VALUE;
      if(device->suppress_zero)
      {
        device->state = STATE_ZERO;
        device->cancel_zero = 4;
      }
      clearchar(device);
      if(device->suppress_plus <= 0)
        output = 1;
      if(output)
        addchar(device, input);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_ZERO:
    switch(input)
    {
    case ' ':
      /* zero suppression */
      if(--device->cancel_zero <= 0)
        device->state = STATE_VALUE;
      break;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    case 'g':
    case '\r':
    case '\n':
    case '\0':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      output = 1;
      complete = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_VALUE:
    switch(input)
    {
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      output = 1;
      break;
    case ' ':
    case 'g':
      break;
    case '\r':
    case '\n':
    case '\0':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      complete = 1;
      output = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  }
  return complete;
}

int key_convert_radwag(struct caliper *device, char input, struct input_event *event)
{
  int output = 0, complete = 0;
  event->type = EV_KEY;
  event->value = 1;

  switch(device->state)
  {
  case STATE_IDLE:
  case STATE_IDLE60S:
  case STATE_1OFF:
  case STATE_2OFF:
  case STATE_ADDR:
  case STATE_SENDING:
  case STATE_STABLE:
  case STATE_B4SIGN:
    switch(input)
    {
    case '-':
      output = 1;
    case '+':
    case ' ':
      device->state = STATE_VALUE;
      printf("*** ENTER STATE_VALUE\n");
      if(device->suppress_zero)
      {
        device->state = STATE_ZERO;
        device->cancel_zero = 4;
      }
      clearchar(device);
      if(device->suppress_plus <= 0)
        output = 1;
      if(output)
        addchar(device, input);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_ZERO:
    switch(input)
    {
    #if 1
    case ' ':
      /* zero suppression */
      if(--device->cancel_zero <= 0)
        device->state = STATE_VALUE;
      break;
    #endif
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      device->state = STATE_VALUE;
      output = 1;
      break;
    case '[':
    case ']':
    case 'g':
    case '\r':
    case '\n':
    case '\0':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      output = 1;
      complete = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  case STATE_VALUE:
    switch(input)
    {
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      addchar(device, input);
      output = 1;
      break;
    case ' ':
    case '[':
    case ']':
    case 'g':
      break;
    case '\r':
    case '\n':
    case '\0':
      addchar(device, '\0');
      device->state = STATE_IDLE;
      complete = 1;
      output = 1;
      // beep(device, 1);
      break;
    default:
      device->state = STATE_IDLE;
      break;
    }
    break;
  }
  return complete;
}

int key_convert(struct caliper *device, char input, struct input_event *event)
{
  switch(device->protocol)
  {
    case PROTOCOL_MITUTOYO:
      return key_convert_mitutoyo(device, input, event);
    case PROTOCOL_DENVER:
      return key_convert_denver(device, input, event);
    case PROTOCOL_METTLERTOLEDO:
      return key_convert_mettlertoledo(device, input, event);
    case PROTOCOL_RADWAG:
      return key_convert_radwag(device, input, event);
    case PROTOCOL_SIMPLE:
      return key_convert_simple(device, input, event);
  }
  return 0;
}

void *keypress_handler(void *data)
{
  struct caliper *device = (struct caliper *) data;
  struct input_event event, scan, syn;
  struct keysequence *ptr;
  char value[VALUE_MAX+2];
  int i, n;

  if(device->sendvalue[0] != '?')
    sprintf(value, "(%s)", device->sendvalue);
  else
    strcpy(value, device->sendvalue);

  n = strlen(value);

  if(n < VALUE_MAX)
  {
    for(i = 0; i < n; i++)
    {
      if(verbose)
        printsequence(device, value[i]);
      /* do the keypress here */
      for(ptr = device->ascii2key[(int) value[i]]; ptr != NULL; ptr = ptr->next)
      {
        event.type = EV_KEY;
        event.code = ptr->key;
        event.value = ptr->press;

        scan.type = EV_MSC;
        scan.code = MSC_SCAN;
        scan.value = event.code;

        syn.type = EV_SYN;
        syn.code = SYN_REPORT;
        syn.value = 0;

        write(device->uifd, &scan, sizeof(struct input_event));
        write(device->uifd, &event, sizeof(struct input_event));
        write(device->uifd, &syn, sizeof(struct input_event));
      }
      if(device->output_delay > 0)
        usleep(device->output_delay); /* slow down generation of key presses */
    }
  }
  clearchar(device);
//  pthread_exit(0);
  return NULL;
}
