#ifndef DRIVER_INIT_H_INCLUDED
#define DRIVER_INIT_H_INCLUDED

#include <compiler.h>
#include <clock_config.h>
#include <port.h>
#include <atmel_start_pins.h>

//#include <clkctrl.h>

//#include <usart_basic.h>

//#include <spi_basic.h>
#include <dac_basic.h>
//#include <vref.h>
#include <tca.h>
//#include <evsys.h>

//#include <cpuint.h>
//#include <slpctrl.h>
//#include <bod.h>

#ifdef __cplusplus
extern "C" {
#endif

void system_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_INIT_H_INCLUDED */
