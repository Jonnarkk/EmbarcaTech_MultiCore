#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "libs/ssd1306.h"
#include "libs/bmp280.h"
#include "pico/binary_info.h"

#define INTERVALO_MS 500
// Endereço I2C do MPU6050 & BMP280
#define I2C_PORT i2c0               // i2c0 pinos 0 e 1, i2c1 pinos 2 e 3
#define I2C_SDA 0                   // 0 ou 2
#define I2C_SCL 1                  // 1 ou 3
              
#define LED_RED 13
#define LED_BLUE 12

// Enum para controlar a coleta de dados do sensores - Alternar leitura
typedef enum ESCOLHA_SENSORES {
    MPU,
    BMP
}ESCOLHA_SENSORES_T;

// Variáveis Globais
ssd1306_t ssd;      // Struct para inicialização do display
static int addr = 0x68;             // O endereço padrao deste IMU é o 0x68
int8_t cont = 0;            // Contador para fazer um MUX entre a leitura dos dois sensores
struct bmp280_calib_param params;   // Estrutura para armazenar os parâmetros de calibração do BMP280
ESCOLHA_SENSORES_T MUX = MPU;             // Estrutura para escolher entre as leituras dos sensores da FIFO 
// Variáveis para armazenar os últimos valores
volatile float ultimo_roll = 0.0f;
volatile float ultima_temp = 0.0f;
volatile bool novo_dado = false;   // indica se há algo novo para atualizar

// Funções para Inicialização do MPU6050
static void mpu6050_reset() {
     // Two byte reset. First byte register, second byte data
     // There are a load more options to set up the device in different ways that could be added here
     uint8_t buf[] = {0x6B, 0x80};
     i2c_write_blocking(I2C_PORT, addr, buf, 2, false);
     sleep_ms(100); // Allow device to reset and stabilize
 
     // Clear sleep mode (0x6B register, 0x00 value)
     buf[1] = 0x00;  // Clear sleep mode by writing 0x00 to the 0x6B register
     i2c_write_blocking(I2C_PORT, addr, buf, 2, false); 
     sleep_ms(10); // Allow stabilization after waking up
 }
 
 static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
     // For this particular device, we send the device the register we want to read
     // first, then subsequently read from the device. The register is auto incrementing
     // so we don't need to keep sending the register we want, just the first.
 
     uint8_t buffer[6];
 
     // Start reading acceleration registers from register 0x3B for 6 bytes
     uint8_t val = 0x3B;
     i2c_write_blocking(I2C_PORT, addr, &val, 1, true); // true to keep master control of bus
     i2c_read_blocking(I2C_PORT, addr, buffer, 6, false);
 
     for (int i = 0; i < 3; i++) {
         accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
     }
 
     // Now gyro data from reg 0x43 for 6 bytes
     // The register is auto incrementing on each read
     val = 0x43;
     i2c_write_blocking(I2C_PORT, addr, &val, 1, true);
     i2c_read_blocking(I2C_PORT, addr, buffer, 6, false);  // False - finished with bus
 
     for (int i = 0; i < 3; i++) {
         gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);;
     }
 
     // Now temperature from reg 0x41 for 2 bytes
     // The register is auto incrementing on each read
     val = 0x41;
     i2c_write_blocking(I2C_PORT, addr, &val, 1, true);
     i2c_read_blocking(I2C_PORT, addr, buffer, 2, false);  // False - finished with bus
 
     *temp = buffer[0] << 8 | buffer[1];
 }

void core1_interrupt_handler()
{
    while (multicore_fifo_rvalid()) {
        float val = multicore_fifo_pop_blocking(); // recebe dado
        if (MUX == MPU) {
            ultimo_roll = val;
            MUX = BMP;

            gpio_put(LED_RED, true);
            gpio_put(LED_BLUE, false);
        } else {
            ultima_temp = val / 100.0f;
            MUX = MPU;

            gpio_put(LED_RED, false);
            gpio_put(LED_BLUE, true);
        }
        novo_dado = true; // sinaliza que tem atualização
    }
    multicore_fifo_clear_irq();
}

void core1_entry()
{
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);

    char texto[32];

    while (true) {
        if (novo_dado) {
            novo_dado = false;

            // Limpa o buffer antes de redesenhar toda a tela
            ssd1306_fill(&ssd, false);

            // Moldura opcional
            ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);

            // Escreve o ROLL
            sprintf(texto, "ROLL: %.1f", ultimo_roll);
            ssd1306_draw_string(&ssd, texto, centralizar_texto(texto), 20);

            // Escreve a TEMPERATURA
            sprintf(texto, "TEMP: %.1f C", ultima_temp);
            ssd1306_draw_string(&ssd, texto, centralizar_texto(texto), 40);

            // Envia tudo de uma vez
            ssd1306_send_data(&ssd);

            // Delay para display
            sleep_ms(50);

        }

        tight_loop_contents();
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events){

    // Reinicia a placa e limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    reset_usb_boot(0, 0);
}

void setup(){
    // Inicialização dos LED's
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, false);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_RED, false);

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    // Inicialização do display
    display_init(&ssd);

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Make the I2C pins available to picotool
    printf("Antes do bi_decl...\n");
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
    printf("Antes do reset MPU...\n");
    mpu6050_reset();

     // Inicializa o BMP280
    bmp280_init(I2C_PORT);
    bmp280_get_calib_params(I2C_PORT, &params);
}

int main(){

    setup();

    stdio_init_all();

    multicore_launch_core1(core1_entry);

    // Variáveis para armazenar os dados do sensor MPU
    int16_t acceleration[3], gyro[3], temp;

    // Variáveis para armazenar os dados do sensor BMP280
    int32_t raw_temp_bmp, raw_pressure;


    while (true)
    {
        // Lê os dados brutos do sensor
        mpu6050_read_raw(acceleration, gyro, &temp);
        
        printf("Core 0: Dados do MPU lidos.\n");

        // Leitura dos dados de aceleração, giroscópio e temperatura
        float ax = acceleration[0] / 16384.0f;
        float ay = acceleration[1] / 16384.0f;
        float az = acceleration[2] / 16384.0f;

        // Cálculo do ângulo em graus (Roll)
        float roll  = atan2(ay, az) * 180.0f / M_PI;

        multicore_fifo_push_blocking(roll); // Envia para Core 1
        sleep_ms(INTERVALO_MS);

         // Leitura do BMP280
        bmp280_read_raw(I2C_PORT, &raw_temp_bmp, &raw_pressure);
        int32_t temperature = bmp280_convert_temp(raw_temp_bmp, &params);
        int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);
        
        multicore_fifo_push_blocking(temperature); // Envia para Core 1
        printf("Core 0: Dados do BMP280 lidos.\n");
        sleep_ms(INTERVALO_MS);
    }
}
