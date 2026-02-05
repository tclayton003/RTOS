/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#include "../local.h"
#include <gs/csp/drivers/i2c/i2c.h>
#include <csp/interfaces/csp_if_kiss.h>
#include <csp/interfaces/csp_if_can.h>
#include <csp/interfaces/csp_if_i2c.h>
#include <csp/interfaces/csp_if_zmqhub.h>
#include <csp/drivers/usart.h>
#include <csp/drivers/can_socketcan.h>
#include <gs/util/linux/function.h>
#include <gs/util/linux/drivers/i2c/i2c.h>
#include <gs/util/string.h>

#define IF_NAME              "if"

#define DEFAULT_CAN_DEVICE   "can0"

#define DEFAULT_KISS_IF_NAME  "KISS"
#define DEFAULT_KISS_DEVICE   "/dev/ttyUSB0"
#define KISS_SPEED            "speed"
#define DEFAULT_KISS_SPEED    500000

#define DEFAULT_ZMQ_SERVER   "localhost"

#define DEFAULT_I2C_DEVICE   "0"

#define RX_FILTER            "rxfilter"

#define CSP_ADDRESS_NOT_SET  255
#define DEFAULT_CSP_ADDRESS  8

#define MAX_DEVICES          2

static uint8_t csp_address = CSP_ADDRESS_NOT_SET;
static const char * csp_can_device;
static const char * csp_kiss_device;
static const char * csp_i2c_device;
static const char * csp_zmq_server[MAX_DEVICES];
static const char * csp_rtable;

static int parser(int key, char *arg, struct argp_state *state)
{
    switch (key) {
        case 'a':
            return gs_string_to_uint8(arg, &csp_address);

        case 'c':
            if (csp_can_device) {
                return GS_ERROR_IN_USE;
            }
            if (arg) {
                csp_can_device = arg;
            } else {
                csp_can_device = DEFAULT_CAN_DEVICE;
            }
            break;

        case 'k':
            if (csp_kiss_device) {
                return GS_ERROR_IN_USE;
            }
            if (arg) {
                csp_kiss_device = arg;
            } else {
                csp_kiss_device = DEFAULT_KISS_DEVICE;
            }
            break;

        case 'i':
            if (csp_i2c_device) {
                return GS_ERROR_IN_USE;
            }
            if (arg) {
                csp_i2c_device = arg;
            } else {
                csp_i2c_device = DEFAULT_I2C_DEVICE;
            }
            break;

        case 'z':
            {
                const char * server = (arg) ? arg : DEFAULT_ZMQ_SERVER;
                unsigned int i;
                for (i = 0; i < MAX_DEVICES; ++i) {
                    if (csp_zmq_server[i] == NULL) {
                        csp_zmq_server[i] = server;
                        break;
                    }
                }
                if (i == MAX_DEVICES) {
                    return GS_ERROR_IN_USE;
                }
            }
            break;

        case 'R':
            csp_rtable = arg;
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

#define CSP_OPTION_ADDRESS {                    \
        .name = "csp-address",                  \
        .key = 'a',                             \
        .arg = "ADDR",                          \
        .flags = 0,                             \
        .doc = "Set CSP address"                \
    }

#define CSP_OPTION_RTABLE {                     \
        .name = "csp-rtable",                   \
        .key = 'R',                             \
        .arg = "RTABLE",                        \
        .flags = 0,                             \
        .doc = "Set routing table\nRTABLE=<address>/<mask> <interface> [mac]\nExample: \"0/0 ZMQHUB 24, 24/2 ZMQHUB\"" \
    }

#define CSP_OPTION_CAN {                        \
        .name = "csp-can",                      \
        .key = 'c',                             \
        .arg = "DEVICE",                        \
        .flags = OPTION_ARG_OPTIONAL,                           \
        .doc = "Add CAN interface\nDEVICE=" DEFAULT_CAN_DEVICE  \
    }

#define CSP_OPTION_KISS {                       \
        .name = "csp-kiss",                     \
        .key = 'k',                             \
        .arg = "DEVICE",                        \
        .flags = OPTION_ARG_OPTIONAL,                                   \
        .doc = "Add KISS over UART interface\nDEVICE=" DEFAULT_KISS_DEVICE "," IF_NAME "=" DEFAULT_KISS_IF_NAME","KISS_SPEED"=" GS_DEF2STRING(DEFAULT_KISS_SPEED)\
    }

#define CSP_OPTION_I2C {                        \
        .name = "csp-i2c",                      \
        .key = 'i',                             \
        .arg = "DEVICE",                        \
        .flags = OPTION_ARG_OPTIONAL,                                   \
        .doc = "Add I2C interface\nDEVICE=0,"GS_I2C_COMMAND_LINE_SPEED"=" GS_DEF2STRING(GS_I2C_DEFAULT_BPS) "," GS_I2C_COMMAND_LINE_ADDRESS "=1," GS_I2C_COMMAND_LINE_DEVICE "=" GS_DEF2STRING(GS_I2C_ALL_DEVICES) \
    }

#define CSP_OPTION_ZMQHUB {                     \
        .name = "csp-zmq",                      \
        .key = 'z',                             \
        .arg = "SERVER",                        \
        .flags = OPTION_ARG_OPTIONAL,                           \
        .doc = "Add ZMQ interface\nSERVER=" DEFAULT_ZMQ_SERVER  "[," RX_FILTER "=<node[:node]>]"\
    }

static const struct argp_option options[] = {
    CSP_OPTION_ADDRESS,
    CSP_OPTION_RTABLE,
#if (CSP_USE_CAN)
    CSP_OPTION_CAN,
#endif
#if (CSP_USE_KISS)
    CSP_OPTION_KISS,
#endif
#if (CSP_USE_I2C)
    CSP_OPTION_I2C,
#endif
#if (CSP_USE_ZMQHUB)
    CSP_OPTION_ZMQHUB,
#endif
    {
        .flags = OPTION_DOC,
        .name = "Examples:"
#if (CSP_USE_CAN)
        "\n  CAN: configure address 10 and CAN interface can0:"
        "\n    $ <application> -a10 -ccan0"
#endif
#if (CSP_USE_KISS)
        "\n  KISS: configure address 10 and uart on /dev/ttyUSB0 at baudrate 500000:"
        "\n    $ <application> -a10 -k/dev/ttyUSB0,speed=500000"
#endif
#if (CSP_USE_I2C)
        "\n  I2C: configure address 10 and I2C Aardvark dongle with id 2238384015, speed 400K:"
        "\n    $ <application> -a10 -i2238384015,speed=400000"
#endif
#if (CSP_USE_ZMQHUB)
        "\n  ZMQ: configure address 10 and ZMQ proxy on localhost:"
        "\n    $ <application> -a10 -zlocalhost"
#endif
    },
    {0}
};

static const struct argp argp = {.options = options, .parser = parser};

const struct argp_child gs_csp_command_line_options = {.argp = &argp, .header = "CSP"};

static const struct argp_option options_devices[] = {
#if (CSP_USE_CAN)
    CSP_OPTION_CAN,
#endif
#if (CSP_USE_KISS)
    CSP_OPTION_KISS,
#endif
#if (CSP_USE_I2C)
    CSP_OPTION_I2C,
#endif
#if (CSP_USE_ZMQHUB)
    CSP_OPTION_ZMQHUB,
#endif
    {0}
};

static const struct argp argp_devices = {.options = options_devices, .parser = parser};

const struct argp_child gs_csp_command_line_options_devices = {.argp = &argp_devices, .header = "CSP"};

gs_error_t gs_csp_command_line_configure_interfaces(void)
{
#if (CSP_USE_KISS)
    // KISS - only here, because the embedded init functions are stubbed in libemul
    if (csp_kiss_device) {
        static char device[GS_PATH_MAX + 1];
        static char ifname[GS_PATH_MAX + 1];
        uint32_t speed;
        int res = gs_string_get_suboption_string(csp_kiss_device, NULL, DEFAULT_KISS_DEVICE, device, sizeof(device));
        res |= gs_string_get_suboption_string(csp_kiss_device, IF_NAME, DEFAULT_KISS_IF_NAME, ifname, sizeof(ifname));
        res |= gs_string_get_suboption_uint32(csp_kiss_device, KISS_SPEED, DEFAULT_KISS_SPEED, &speed);
        if (res) {
            log_error("Failed to parse command line: [%s], res: %d", csp_kiss_device, res);
            return GS_ERROR_ARG;
        }

        static csp_iface_t csp_if_kiss;
        static csp_kiss_handle_t csp_kiss_driver;
        csp_kiss_init(&csp_if_kiss, &csp_kiss_driver, usart_putc, usart_insert, ifname);

        struct usart_conf conf = {.device = device, .baudrate = speed};
        res = usart_init(&conf);
        if (res != CSP_ERR_NONE) {
            log_error("Failed to initialize UART, command line: [%s], res: %d", csp_kiss_device, res);
            return GS_ERROR_IO;
        }

        void my_usart_rx(uint8_t * buf, int len, void * pxTaskWoken) {
            csp_kiss_rx(&csp_if_kiss, buf, len, pxTaskWoken);
        }
        usart_set_callback(my_usart_rx);
    }
#endif

#if (CSP_USE_CAN)
    // CAN - only here, because the embedded init functions are stubbed in libemul
    if (csp_can_device) {
        char device[GS_PATH_MAX + 1];
        int res = gs_string_get_suboption_string(csp_can_device, NULL, DEFAULT_CAN_DEVICE, device, sizeof(device));
        if (res) {
            log_error("Failed to parse command line: [%s], res: %d", csp_kiss_device, res);
            return GS_ERROR_ARG;
        }

        csp_iface_t * can = csp_can_socketcan_init(device, 0, (csp_get_address() == 255) ? 1 : 0);
        if (can == NULL) {
            log_error("Failed to initialize CAN, command line: [%s], res: %d", csp_can_device, res);
            return GS_ERROR_IO;
        }
    }
#endif

#if (CSP_USE_ZMQHUB)
    // ZMQ - currently ZMQ is only supported on Linux, and therefor handled here
    for (unsigned int i = 0; i < MAX_DEVICES; ++i) {
        if (csp_zmq_server[i] == NULL) {
            continue;
        }
        char server[GS_PATH_MAX + 1];
        char ifname[GS_PATH_MAX + 1];
        char rxfilter_str[GS_PATH_MAX + 1];
        int res = gs_string_get_suboption_string(csp_zmq_server[i], NULL, DEFAULT_ZMQ_SERVER, server, sizeof(server));
        res |= gs_string_get_suboption_string(csp_zmq_server[i], IF_NAME, CSP_ZMQHUB_IF_NAME, ifname, sizeof(ifname));
        res |= gs_string_get_suboption_string(csp_zmq_server[i], RX_FILTER, "", rxfilter_str, sizeof(rxfilter_str));
        if (res) {
            log_error("Failed to parse command line: [%s], res: %d", csp_zmq_server[i], res);
            return GS_ERROR_ARG;
        }

        if (gs_string_empty(rxfilter_str) && gs_csp_is_address_valid(csp_get_address())) {
            snprintf(rxfilter_str, sizeof(rxfilter_str), "%u", csp_get_address());
        }

        char pub[GS_PATH_MAX + 1];
        csp_zmqhub_make_endpoint(server, CSP_ZMQPROXY_SUBSCRIBE_PORT, pub, sizeof(pub));

        char sub[GS_PATH_MAX + 1];
        csp_zmqhub_make_endpoint(server, CSP_ZMQPROXY_PUBLISH_PORT, sub, sizeof(sub));

        const unsigned int MAX_RX_FILTERS = 50;
        uint8_t rxfilter[MAX_RX_FILTERS];
        unsigned int rxfilter_count = 0;
        {
            char * saveptr = NULL;
            char * filter = strtok_r(rxfilter_str, ":", &saveptr);
            while (filter && (rxfilter_count < MAX_RX_FILTERS)) {
                if (gs_string_to_uint8(filter, &rxfilter[rxfilter_count]) == GS_OK) {
                    ++rxfilter_count;
                }

                filter = strtok_r(NULL, ":", &saveptr);
            }
        }

        res = csp_zmqhub_init_w_name_endpoints_rxfilter(ifname, rxfilter, rxfilter_count, pub, sub, NULL);
        if (res != CSP_ERR_NONE) {
            log_error("Failed to initialize ZMQ, command line: [%s], res: %d", csp_zmq_server[i], res);
            return GS_ERROR_IO;
        }
    }
#endif

#if (CSP_USE_I2C)
    // I2C
    if (csp_i2c_device) {
        uint8_t device = 0;
        int res = gs_string_get_suboption_uint8(csp_i2c_device, GS_I2C_COMMAND_LINE_DEVICE, GS_I2C_ALL_DEVICES, &device);
        if (res) {
            log_error("Failed to parse command line: [%s], res: %d", csp_i2c_device, res);
            return GS_ERROR_ARG;
        }

        if (device == GS_I2C_ALL_DEVICES) {
            device = 0;
        }

        char modified_options[GS_PATH_MAX + 100];
        snprintf(modified_options, sizeof(modified_options), "%s,%s=%u", csp_i2c_device, GS_I2C_COMMAND_LINE_ADDRESS, csp_get_address());
        res = gs_function_invoke("i2c", modified_options);
        if (res) {
            log_error("Failed to initialize I2C adapter: [%s], error: %d", modified_options, res);
            return res;
        }

        res = gs_csp_i2c_init(device, csp_get_address());
        if (res) {
            log_error("gs_csp_i2c_init(%u, %u) failed, error: %d", device, csp_get_address(), res);
            return res;
        }
    }
#endif

    return GS_OK;
}

bool gs_csp_command_line_is_address_set(void)
{
    return (csp_address != CSP_ADDRESS_NOT_SET);
}

uint8_t gs_csp_command_line_get_address(void)
{
    if (gs_csp_command_line_is_address_set()) {
        return csp_address;
    }
    return DEFAULT_CSP_ADDRESS;
}

const char * gs_csp_command_line_get_rtable(void)
{
    return csp_rtable;
}
