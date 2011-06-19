#ifndef KNOBS_H
#define KNOBS_H

#include <stdbool.h>
#include <inttypes.h>

extern volatile uint8_t knobs[3];
extern void (*adchandler)(void);
void knobs_adchandler (void) __attribute__ ((signal,__INTR_ATTRS));
void knobs_init(void);

#endif //KNOBS_H
