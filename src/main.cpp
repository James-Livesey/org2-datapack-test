#include <stdio.h>
#include "pico/stdlib.h"

#define LED 0

int main() {
    stdio_init_all();

    printf("Hello, world!\n");

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    while (true) {
        gpio_put(LED, 1);
        printf("On!\n");
        sleep_ms(500);

        gpio_put(LED, 0);
        printf("Off!\n");
        sleep_ms(500);
    }

    return 0;
}