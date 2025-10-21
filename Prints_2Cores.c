/*
 * Por: Wilton Lacerda Silva
 *    Utilização do multicore do RP2040
 *
 *  Este programa imprime mensagens diferentes em cada core.
*/


#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

void core1_entry() {
    while (true) {
        printf("Mensagem do Core 1\n");
        sleep_ms(1000);
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B


    stdio_init_all();
    multicore_launch_core1(core1_entry);

    while (true) {
        printf("Mensagem do Core 0\n");
        sleep_ms(1000);
    }
}
