#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"
#include "hardware/clocks.h"

#include "lwip/apps/http_client.h"

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

static err_t headers_done_fn(httpc_state_t *connection, void *arg,
                             struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    printf("in headers_done_fn\n");
    return ERR_OK;
}

static void result_fn(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    printf("in result_fn\n");
}

static err_t recv_fn(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf("in recv_fn\n");
    printf("payload: %p - \n", p->payload);

    for (int i = 0; i < p->len; i++) {
        printf("%c", ((char*)p->payload)[i]);
    }

    printf("\n");
    return ERR_OK;
}

static void send() {
    httpc_connection_t settings = {
        .use_proxy = 0,
        .result_fn = result_fn,
        .headers_done_fn = headers_done_fn
    };
    static httpc_state_t *connection = NULL;

    err_t err = httpc_get_file_dns("frogfind.com", HTTP_DEFAULT_PORT, "/", &settings, recv_fn, NULL, &connection);
    printf("err = %d\n", err);
}

int main() {
    set_sys_clock_khz(240000, true);

    stdio_init_all();

    printf("Hello, world!\n");

    uint64_t timeSinceLastReport = 0;

    // irq_set_mask_enabled(0x0F, false);
    multicore_launch_core1(stepper);

    sleep_ms(3000);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_UK)) {
        printf("Failed to init CYW43\n");
        return 1;
    }

    printf("CYW43 initialised - %s %s\n", WIFI_SSID, WIFI_PSK);

    cyw43_arch_enable_sta_mode();

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PSK, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi connection error\n");
        return 1;
    }

    printf("Connected to Wi-Fi\n");

    send();

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