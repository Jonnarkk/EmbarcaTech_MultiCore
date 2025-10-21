/*
 * Por: Wilton Lacerda Silva
 *    Utilização do multicore do RP2040
 *
 * Este programa pisca dois LEDs, um no Core 0 e outro no Core 1.
 * O LED Verde pisca no Core 0 e o LED Azul pisca no Core 1.
 *  Os LEDs estão conectados aos pinos 11 e 12.
 *  O LED Verde pisca a cada 500 ms e o LED Azul a cada 700 ms.
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"

#define LED_VERDE 11 // GPIO do LED Verde
#define LED_AZUL 12  // GPIO do LED Azul
#define T_core1 1600  // Tempo de piscar do LED Azul
#define T_core0 500  // Tempo de piscar do LED Verde

// Função executada no Core 1 (piscando o LED Azul)
void core1_entry()
{
    while (1)
    {
        gpio_put(LED_AZUL, 1); // Liga LED Azul
        sleep_ms(T_core1);
        gpio_put(LED_AZUL, 0); // Desliga LED Azul
        sleep_ms(T_core1);
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

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);

    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);

    // Inicia o Core 1
    multicore_launch_core1(core1_entry);

    while (1)
    {
        gpio_put(LED_VERDE, 1); // Liga LED Verde
        sleep_ms(T_core0);
        gpio_put(LED_VERDE, 0); // Desliga LED Verde
        sleep_ms(T_core0);
    }
}
