#ifndef E002F9B9_INPUT_H
#define E002F9B9_INPUT_H

#include <stdint.h>
#include <zephyr/kernel.h>
#include <autoconf.h>
#include <zephyr/drivers/uart.h>

///custom implementation of console input
///example:
///```
///const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
///char line[CONFIG_CONSOLE_INPUT_MAX_LINE_LEN];
///getline_uart(uart_dev, line, sizeof(line));
///```
///contents of input will be inside the variable: line
void getline_uart(const struct device *dev, char *buf, size_t max) {
    size_t i = 0;
    unsigned char c;
    while (i < max - 1) {
        if (uart_poll_in(dev, &c) == 0) {
            if (c == '\n' || c == '\r') break;   // enter -> finish
            if ((c == '\b' || c == 0x7F) && i > 0) {
                i--;
                uart_poll_out(dev, '\b');
                uart_poll_out(dev, ' ');
                uart_poll_out(dev, '\b');
                continue;
            }
            buf[i++] = c;
            uart_poll_out(dev, c);
        }
    }
    buf[i] = '\0'; //last character must be null to terminate array
    uart_poll_out(dev, '\r');
    uart_poll_out(dev, '\n');
}

// /// note: not tested
// bool getline_uart_timed(const struct device *dev, char *buf, size_t max, int timeout_ms) {
//     size_t i = 0;
//     unsigned char c;
//     int64_t time_start = k_uptime_get();
//     while (i < max - 1) {
//         if (uart_poll_in(dev, &c) == 0) {
//             if (c == '\n' || c == '\r') break;   // enter -> finish
//             if ((c == '\b' || c == 0x7F) && i > 0) {
//                 i--;
//                 uart_poll_out(dev, '\b');
//                 uart_poll_out(dev, ' ');
//                 uart_poll_out(dev, '\b');
//             } else {
//                 buf[i++] = c;
//                 uart_poll_out(dev, c);
//             }
//         }
//         if (timeout_ms > 0 && (k_uptime_get() - time_start) >= timeout_ms) {
//             buf[i] = '\0';
//             return false; // timed out
//         }
//     }
//     buf[i] = '\0'; //last character must be null to terminate array
//    uart_poll_out(dev, '\r');
//    uart_poll_out(dev, '\n');
//     return true;
// }

#endif /* E002F9B9_INPUT_H */