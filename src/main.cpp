#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "datapack.h"

#define PIN_SD 16, 15, 17, 14, 18, 13, 19, 12
#define PIN_SMR 11
#define PIN_SCLK 20
#define PIN_SOE_B 10
#define PIN_SS_B 21
#define PIN_SPGM_B 22

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

    irq_set_mask_enabled(0x0F, false);
    multicore_launch_core1(stepper);

    while (true) {
        char key;

        printf(">");
        scanf("%c", &key);
        printf("%c\n", key);

        if (key == 't') {
            printf("Tick %d\n", datapack->tick);
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