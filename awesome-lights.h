#ifndef AWESOME_LIGHTS_H
#define AWESOME_LIGHTS_H

#include <inttypes.h>
#include <stdbool.h>

#define F_CPU 8000000UL /* Clock Frequency = 8Mhz */

typedef enum {RED, GREEN, BLUE, NONE} COLOR;
#define DIODE_PORT PORTD
#define LAMP_COUNT 20
#define TIMESLICE 255

extern uint8_t diode_config[20][3];
#define diode_config_p ((uint8_t*)&diode_config)
#define set_diode_cfg(i,c,v) {diode_config_p[(uint8_t)(3*(i)+(c))] = \
	TIMESLICE*(v) >> 8;}

extern volatile bool program_running;

#endif /*AWESOME_LIGHT_H*/
