#ifndef E002F9B9_DEVICE_INIT_H
#define E002F9B9_DEVICE_INIT_H

#define E002F9B9_device_init() \
    port_set_dw_ic_spi_fastrate();\
    reset_DWIC();\
    Sleep(2); \
    while (!dwt_checkidlerc()) { /* spin */ };\
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)\
    {\
        LOG_ERR("INIT FAILED");\
        while (1) { /* spin */ };\
    }\
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK) ;\
    if (dwt_configure(&config)) \
    {\
        LOG_ERR("CONFIG FAILED");\
        while (1) { /* spin */ };\
    }\
    dwt_configuretxrf(&txconfig_options);\


#endif /* E002F9B9_DEVICE_INIT_H */