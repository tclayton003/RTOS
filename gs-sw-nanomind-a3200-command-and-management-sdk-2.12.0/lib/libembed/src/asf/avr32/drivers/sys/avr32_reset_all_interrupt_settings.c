/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include <gs/embed/asf/drivers/i2c/i2c.h>
#include <canif.h>
#include <twim.h>
#include <pdca.h>
#include <gs/embed/asf/avr32/drivers/gpio/gpio.h>

/**
   Reset and disable all interrupts to avoid problems when booting RAM image
*/
void gs_sys_avr32_reset_all_interrupt_settings(void)
{
	/* Reset CAN in case of software jump */
	CANIF_disable_wakeup_interrupt(0);
	CANIF_disable_interrupt(0);
	CANIF_disable(0);

	/* Reset I2C in case of software jump */
	twim_reset(0);
	twim_disable_interrupt(0);
	twim_reset(2);
	twim_disable_interrupt(2);
	twis_reset(0);

	/* Reset and disable all PDCA channels (Peripheral DMA Controller) in case of software jump */
	/* This also covers the UART, since it is setup to use PDCA*/
	for (int i = 0; i < AVR32_PDCA_CHANNEL_LENGTH; i++) {
		volatile avr32_pdca_channel_t *pdca_channel = &AVR32_PDCA.channel[i];
		pdca_channel->idr = ~0UL;
		pdca_channel->isr;
		pdca_channel->cr = AVR32_PDCA_TDIS_MASK;
	}

    /* Disable all GPIO interrupts and clear any pending */
	{
	const uint32_t ALL = 0xFFFFFFFF;
	volatile avr32_gpio_port_t *gpio_port = &AVR32_GPIO.port[GS_AVR32_GPIO_PORT_A];
	gpio_port->ierc = ALL; // disable
	gpio_port->ifrc = ALL; // clear
	gpio_port = &AVR32_GPIO.port[GS_AVR32_GPIO_PORT_B];
	gpio_port->ierc = ALL; // disable
	gpio_port->ifrc = ALL; // clear
	gpio_port = &AVR32_GPIO.port[GS_AVR32_GPIO_PORT_C];
	gpio_port->ierc = ALL; // disable
	gpio_port->ifrc = ALL; // clear
	gpio_port = &AVR32_GPIO.port[GS_AVR32_GPIO_PORT_D];
	gpio_port->ierc = ALL; // disable
	gpio_port->ifrc = ALL; // clear
	}

}

