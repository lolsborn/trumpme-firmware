#include "driver_init.h"
#include "clkctrl.h"
#include "slpctrl.h"
#include <system.h>


/**
 * \brief System initialization
 */
void system_init()
{
	mcu_init();

	//CLKCTRL_init();

	playback_button_set_pull_mode(PORT_PULL_UP); // Button is active low
	
	//TIMER_0_init();

	DAC_0_init();

	//CPUINT_init();

	//SLPCTRL_init();

	//BOD_init();
}
