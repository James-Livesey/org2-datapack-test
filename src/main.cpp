#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"

#include "datapack.h"

#define TOP_SLOT

#ifdef TOP_SLOT
    #define PIN_SD 15, 14, 13, 12, 4, 5, 6, 7
    #define PIN_SMR 8
    #define PIN_SCLK 11
    #define PIN_SOE_B 3
    #define PIN_SS_B 10
    #define PIN_SPGM_B 21
#else
    #define PIN_SD 16, 15, 17, 14, 18, 13, 19, 12
    #define PIN_SMR 11
    #define PIN_SCLK 20
    #define PIN_SOE_B 10
    #define PIN_SS_B 21
    #define PIN_SPGM_B 22
#endif

#define PIN_BUS {PIN_SD, PIN_SMR, PIN_SCLK, PIN_SOE_B, PIN_SS_B, PIN_SPGM_B}

Datapack* datapack = new Datapack(PIN_BUS);
bool datapackInUse = false;
bool datapackBlocked = false;

void stepper() {
    while (true) {
        while (datapackBlocked) {
            printf("Blocked...\n");
        }

        datapackInUse = true;
        datapack->step();
        datapackInUse = false;
    }
}

int main() {
    stdio_init_all();

    printf("Hello, world!\n");

    uint64_t timeSinceLastReport = 0;

    set_sys_clock_khz(260000, true);
    irq_set_mask_enabled(0x0F, false);
    multicore_launch_core1(stepper);

    while (true) {
        char key;

        printf(">");
        scanf("%c", &key);
        printf("%c\n", key);

        if (key == 's') {
            datapack->tick = 0;
            datapack->flips = 0;

            printf("Reset tick/flips\n");
        }

        if (key == 't') {
            printf("Tick: %d\n", datapack->tick);
            printf("Flips: %d\n", datapack->flips);
        }

        if (key == 'r') {
            datapackBlocked = true;

            while (datapackInUse) {
                printf("Waiting for finish...\n");
            }

            delete datapack;

            datapack = new Datapack(PIN_BUS);

            datapackBlocked = false;
        }

        if (key == 'm') {
            datapack->dumpMemory();
        }
    }

    return 0;
}