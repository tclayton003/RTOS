#ifndef LIBEMBED_CONF_FREERTOS_PORTMACRO_H
#define LIBEMBED_CONF_FREERTOS_PORTMACRO_H
// dummy - only fo compile check

#include <stdint.h>

#define portBYTE_ALIGNMENT 4

typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef UBaseType_t StackType_t;
typedef UBaseType_t TickType_t;

#define portBASE_TYPE       BaseType_t
#define portMAX_DELAY       ((TickType_t)-1)
#define portTICK_PERIOD_MS  1

#define portENTER_CRITICAL()
#define portEXIT_CRITICAL()

#endif
