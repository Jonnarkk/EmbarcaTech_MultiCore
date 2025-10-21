/*
 * Por: Wilton Lacerda Silva
 *    Utilização do multicore do RP2040
 *
 *    Este programa lê um valor do ADC no Core 0 e envia para o Core 1,
 *    que então imprime o valor recebido e a voltagem correspondente.
 * 
 *    Entretanto, aqui será utilizado uma interrupção no Core 1 para tratar
 *    os dados recebidos do Core 0.
 * 
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

void core1_interrupt_handler()
{
    // É executado quando há dados na FIFO do Core 1
    while (multicore_fifo_rvalid())
    {
        uint32_t val = multicore_fifo_pop_blocking(); // Recebe dado do Core 0
        float voltage = val * 3.3 / 4095.0;           // Converte valor ADC (12 bits) para voltagem
        printf("Core 1 (IRQ): Valor RECEBIDO do Core 0: %lu, Voltagem: %.2f V\n", val, voltage);
    }

    multicore_fifo_clear_irq(); // Limpa a interrupção
}

void core1_entry()
{
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);
    while (true)
    {
       tight_loop_contents();
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
    adc_init();
    adc_gpio_init(26);   // Configura GPIO26 como entrada ADC
    adc_select_input(0); // Canal ADC 0 (GPIO26)
    multicore_launch_core1(core1_entry);

    while (true)
    {
        uint16_t adc_value = adc_read(); // Lê ADC (12 bits)
        printf("Core 0: Valor lido no (ADC): %u\n", adc_value);
        multicore_fifo_push_blocking(adc_value); // Envia para Core 1
        sleep_ms(1000);
    }
}
