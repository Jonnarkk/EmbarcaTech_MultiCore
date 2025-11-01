# Comunicação Multi-core com Interrupção (RP2040)

Este projeto demonstra a utilização dos dois núcleos (cores) do microcontrolador **Raspberry Pi RP2040** (Pico, Pico W, etc.) para realizar tarefas de forma paralela.
Aqui tem quatro exemplos, sendo que um deles utiliza a **FIFO multi-core** para comunicação e tratamento de dados no Core 1 via **interrupção (IRQ)**.

##  Funcionalidade

O programa realiza as seguintes ações:

1.  **Core 0 (Principal):**
    * Lê um valor dos sensores **BMP280** e **MPU6050** a cada meio segundo pelo barramento _I2C0_.
    * Imprime que o valor foi no terminal.
    * Envia o valor dos sensores para o Core 1 através da **FIFO multi-core**.
2.  **Core 1 (Secundário):**
    * Aguarda passivamente.
    * Possui um *handler* de interrupção (`core1_interrupt_handler`) que é acionado **automaticamente** quando novos dados chegam na sua FIFO.
    * Ao ser interrompido, o *handler* lê o valor dos sensores da FIFO.
    * Imprime os valores no terminal e também exibe-os no display OLED, além de indicar qual sensor foi lido através do LED RGB (Vermelho/MPU - Azul/BMP280).

O principal diferencial é o uso da interrupção (`SIO_IRQ_PROC1`) no Core 1, que apenas repassa o valor lido dos sensores para uma variável global, permitindo que o Core 1 tenha seu total processamento na exibição dos dados de forma intuitiva para o usuário.