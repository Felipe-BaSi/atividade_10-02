# Atividade Conversores A/D
## Felipe Barbosa Silveira 

Este projeto utiliza um joystick para controlar a posição de um quadrado em um display OLED e ajustar o brilho de LEDs RGB com base nos valores do joystick. O código foi desenvolvido para ser executado em um microcontrolador Raspberry Pi Pico W.

## Funcionalidades

- **Controle de um quadrado no display OLED**: O joystick move um quadrado na tela do display OLED. Quando o joystick é solto, o quadrado retorna ao centro.
- **Ajuste de brilho dos LEDs RGB**: O brilho dos LEDs RGB é ajustado com base nos valores dos eixos X e Y do joystick.
- **Botões de controle**:
  - Botão A: Liga/desliga os LEDs Vermelho e Azul.
  - Botão do joystick: Alterna entre bordas normais e bordas mais espessas no display OLED e liga o led Verde.

## Componentes Utilizados

- Raspberry Pi Pico
- Display OLED SSD1306 (128x64 pixels)
- Joystick analógico
- LEDs RGB

## Conexões

- **Joystick**:
  - Eixo X: GPIO 27 (Canal ADC 1)
  - Eixo Y: GPIO 26 (Canal ADC 0)
  - Botão: GPIO 22
- **Display OLED**:
  - SDA: GPIO 14
  - SCL: GPIO 15
- **LEDs RGB**:
  - LED Vermelho: GPIO 13
  - LED Verde: GPIO 11
  - LED Azul: GPIO 12
- **Botão A**: GPIO 5

## Como Executar

1. Conecte os componentes conforme descrito acima.
2. Compile e carregue o código no Raspberry Pi Pico.
3. O display OLED exibirá um quadrado controlado pelo joystick.
4. Use os botões para alternar funcionalidades adicionais (Desativar leds azul e vermelho e ativar led verde).

## Estrutura do Código

- **Inicialização**: Configuração dos pinos, ADC, I2C, PWM e display OLED.
- **Loop Principal**:
  - Leitura dos valores do joystick.
  - Atualização da posição do quadrado no display.
  - Ajuste do brilho dos LEDs RGB.
  - Tratamento de interrupções dos botões.
