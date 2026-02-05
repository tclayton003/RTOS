/*
  FreeRTOS.org V5.1.1 - Copyright (C) 2003-2008 Richard Barry.

  This file is part of the FreeRTOS.org distribution.

  FreeRTOS.org is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  FreeRTOS.org is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FreeRTOS.org; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  A special exception to the GPL can be applied should you wish to distribute
  a combined work that includes FreeRTOS.org, without being obliged to provide
  the source code for any proprietary components.  See the licensing section
  of http://www.FreeRTOS.org for full details of how and when the exception
  can be applied.

  ***************************************************************************
  ***************************************************************************
  *                                                                         *
  * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
  * and even write all or part of your application on your behalf.          *
  * See http://www.OpenRTOS.com for details of the services we provide to   *
  * expedite your project.                                                  *
  *                                                                         *
  ***************************************************************************
  ***************************************************************************

  Please ensure to read the configuration and relevant port sections of the
  online documentation.

  http://www.FreeRTOS.org - Documentation, latest information, license and
  contact details.

  http://www.SafeRTOS.com - A version that is certified for use in safety
  critical systems.

  http://www.OpenRTOS.com - Commercial support, development, porting,
  licensing and training services.
*/


/* BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER FOR IAR AVR PORT. */

#include <gs/embed/asf/drivers/usart/usart.h>

#include <stdlib.h>
#include <ctype.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <gs/embed/freertos.h>
#include <gs/util/log.h>
#include <gs/util/minmax.h>
#include <conf_asf.h> // USART_CONSOLE

#if (GS_ASF_USART_CONSOLE == 0)
#define MY_SIG_DATA USART0_UDRE_vect
#define MY_SIG_RECV USART0_RX_vect
#define UCSRA	UCSR0A
#define UCSRB	UCSR0B
#define UCSRC	UCSR0C
#define UBRRL	UBRR0L
#define UBRRH	UBRR0H
#define UDR		UDR0
#define UDRADDR 0XC6
#elif (GS_ASF_USART_CONSOLE == 1)
#define MY_SIG_DATA USART1_UDRE_vect
#define MY_SIG_RECV USART1_RX_vect
#define UCSRA	UCSR1A
#define UCSRB	UCSR1B
#define UCSRC	UCSR1C
#define UBRRL	UBRR1L
#define UBRRH	UBRR1H
#define UDR		UDR1
#define UDRADDR 0XCE
#else
#error "Unsupported console port"
#endif

#define serBAUD_DIV_CONSTANT			( ( unsigned portLONG ) 8 )

/* Constants for writing to UCSRB. */
#define serRX_INT_ENABLE				( ( unsigned portCHAR ) 0x80 )
#define serRX_ENABLE					( ( unsigned portCHAR ) 0x10 )
#define serTX_ENABLE					( ( unsigned portCHAR ) 0x08 )
#define serTX_INT_ENABLE				( ( unsigned portCHAR ) 0x20 )

/* Constants for writing to UCSRC. */
#define serUCSRC_SELECT					( ( unsigned portCHAR ) 0x80 )
#define serEIGHT_DATA_BITS				( ( unsigned portCHAR ) 0x06 )

static xQueueHandle usart_rxqueue = NULL;
static usart_callback_t usart_callback = NULL;

static int prv_usart_putchar(char c, FILE *stream)
{
    usart_putc(0, c); // handle isn't used - see impl. further down
    return 0;
}

static int prv_usart_getchar(FILE *stream)
{
    return usart_getc(0);  // handle isn't used - see impl. further down
}

void usart_init(int handle, uint32_t fcpu, uint32_t usart_baud)
{
    unsigned portLONG ulBaudRateCounter;
    unsigned portCHAR ucByte;
    unsigned int baud_divider;

    /* Create the queues used by the com test task. */
    usart_rxqueue = xQueueCreate(16, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );

    if (fcpu < 8000000) {
        UCSRA |= _BV(U2X0);
        baud_divider = 8;
    } else {
        baud_divider = 16;
    }

    /* Calculate the baud rate register value from the equation in the
       data sheet. */
    ulBaudRateCounter = ( fcpu / ( baud_divider * usart_baud ) ) - ( unsigned portLONG ) 1;

    /* Set the baud rate. */
    ucByte = ( unsigned portCHAR ) ( ulBaudRateCounter & ( unsigned portLONG ) 0xff );
    UBRRL = ucByte;

    ulBaudRateCounter >>= ( unsigned portLONG ) 8;
    ucByte = ( unsigned portCHAR ) ( ulBaudRateCounter & ( unsigned portLONG ) 0xff );
    UBRRH = ucByte;

    /* Enable the Rx interrupt. Also enable the Rx and Tx. */
    UCSRB |= _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);

    /* set to 8 data bits, 1 stop bit */
    UCSRB &= ~(1 << UCSZ02);
    UCSRC |= (1 << UCSZ01) | (1 << UCSZ00);
    UCSRC &= ~((1 << UMSEL01) | (1 << UMSEL00) | (1 << UPM01) | (1 << UPM00) | (1 << USBS0));

    static FILE mystdout = FDEV_SETUP_STREAM(prv_usart_putchar, prv_usart_getchar, _FDEV_SETUP_RW);
    stdout = &mystdout;
    stdin = &mystdout;

}

void usart_set_callback(int handle, usart_callback_t callback)
{
    usart_callback = callback;
}

char usart_getc(int handle)
{
    unsigned char c;
    xQueueReceive(usart_rxqueue, &c, portMAX_DELAY);
    return c;

}

char usart_getc_nblock(int handle)
{
    unsigned char c =0;
    xQueueReceive(usart_rxqueue, &c, 0);
    return c;

}

void usart_putc(int handle, char c)
{
    /* Spin lock */
    while(!(UCSRA & (1 << UDRE0)))
        continue;

    UDR = c;

}

void usart_putstr(int handle, const char *buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        usart_putc(handle, buf[i]);
    }
}

void usart_insert(int handle, char c, void * pxTaskWoken)
{
    if (pxTaskWoken == NULL)
        xQueueSendToBack(usart_rxqueue, &c, 0);
    else
        xQueueSendToBackFromISR(usart_rxqueue, &c, pxTaskWoken);
}

void MY_SIG_RECV(void) __attribute__((signal));
void MY_SIG_RECV(void)
{
    static signed portBASE_TYPE xTaskWoken;
    static unsigned char c;

    xTaskWoken = pdFALSE;
    c = UDR;

    if (usart_callback != NULL) {
        (*usart_callback)(&c, 1, &xTaskWoken);
    } else {
        xQueueSendToBackFromISR(usart_rxqueue, &c, &xTaskWoken);
    }

    if (xTaskWoken == pdTRUE)
        taskYIELD();
}

int usart_messages_waiting(int handle)
{
    return uxQueueMessagesWaiting(usart_rxqueue);
}

gs_error_t usart_getc_timed(int handle, int timeout_ms, char * ch)
{
    {

        TickType_t ticks = 0;
        if (timeout_ms > 0) {
            ticks = gs_max(timeout_ms / portTICK_RATE_MS, 1U);
        } else if (timeout_ms < 0) {
            ticks = portMAX_DELAY;
        }

        if (xQueueReceive(usart_rxqueue, ch, ticks)) {
            return GS_OK;
        }

        return GS_ERROR_TIMEOUT;
    }

    return GS_ERROR_HANDLE;
}
