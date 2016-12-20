#ifndef CALIPER_KEYPRESS_H
#define CALIPER_KEYPRESS_H

#define TIMEOUT 3000
#define WAITINBETWEEN 500000

extern pthread_mutex_t addchar_mutex;

const char *key2string(int key);
int string2key(char *name);
void clearsequence(struct caliper *device, char origin);
void addsequence(struct caliper *device, char origin, int key, int press);
void pressrelease(struct caliper *device, char origin, int key);
void addkeypress(struct caliper *device, char origin, char *key);
void addkeysequence(struct caliper *device, char origin, char *string);
void setkeysequence(struct caliper *device, char origin, char *string);
void printkey(struct keysequence *ptr);
void printsequence(struct caliper *device, char i);
void printmapping(struct caliper *device);
int clearchar(struct caliper *device);
int addchar(struct caliper *device, char input);
int key_convert(struct caliper *device, char input, struct input_event *event);
void *keypress_handler(void *data);

#endif
