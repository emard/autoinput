#ifndef CALIPER_BEEP_H
#define CALIPER_BEEP_H

extern pthread_mutex_t beep_mutex;

int pcspeaker_register(struct caliper *device);
int pcspeaker_unregister(struct caliper *device);
void set_beep(struct caliper *device, int freq);
void *beep_handler(void *data);
void beep(struct caliper *device, int type);
#endif
