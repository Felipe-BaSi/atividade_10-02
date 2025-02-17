#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "pico/stdio_usb.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Definições para comunicação I2C e display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Definições dos pinos do joystick
#define JOYSTICK_X 27
#define JOYSTICK_Y 26

// Definição dos LEDs RGB
const uint led_g = 11;
const uint led_b = 12;
const uint led_r = 13;

// Variáveis de estado
bool led_g_on = false, A_press = false, joystick_press = false;
const uint8_t square_size = 8;

// Estrutura para o display OLED
ssd1306_t ssd;

// Definição dos botões
const uint but_A = 5;
const uint but_joystick = 22;

// Variável para controle de debounce
volatile uint32_t last_time = 0;

// Inicializa um pino GPIO como saída PWM
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);  
    return slice_num;
}
  
// Desenha um quadrado no display OLED
void draw_square(ssd1306_t *ssd, uint8_t x, uint8_t y, uint8_t size, bool cor) {
    for (uint8_t i = 0; i < size; i++) {
        for (uint8_t j = 0; j < size; j++) {
            ssd1306_pixel(ssd, x + i, y + j, cor);
        }
    }
}

// Verifica se o joystick está na posição central
bool is_joystick_in_rest_position(uint16_t adc_x, uint16_t adc_y) {
    const uint16_t rest_posit_range = 500;
    return (abs(adc_x - 2048) < rest_posit_range && abs(adc_y - 2048) < rest_posit_range);
}

// Atualiza a posição do quadrado com base no movimento do joystick
void update_square_position(uint16_t adc_x, uint16_t adc_y, uint8_t *square_x, uint8_t *square_y, bool joystick_released) {
    if (joystick_released) {
        *square_x = 60;
        *square_y = 28;
    } else {
        int16_t delta_x = (adc_x - 2048) / 256;
        int16_t delta_y = (adc_y - 2048) / 256;
        *square_x += delta_x;
        *square_y -= delta_y;
        
        // Impede que o quadrado saia dos limites da tela
        if (*square_x < square_size) {
            *square_x = square_size;
        } else if (*square_x >= 128 - (2 * square_size)) {
            *square_x = 128 - (2 * square_size);
        }
        if (*square_y < square_size) {
            *square_y = square_size;
        } else if (*square_y >= 64 - (2 * square_size)) {
            *square_y = 64 - (2 * square_size);
        }
    }
}

// Protótipo da função de interrupção do GPIO
static void gpio_irq_handler(uint gpio, uint32_t events);

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(JOYSTICK_Y);
    adc_gpio_init(JOYSTICK_X);
    
    // Configuração dos LEDs e botões
    gpio_init(led_g);
    gpio_set_dir(led_g, GPIO_OUT);
    gpio_init(but_joystick);
    gpio_set_dir(but_joystick, GPIO_IN);
    gpio_pull_up(but_joystick);
    gpio_init(but_A);
    gpio_set_dir(but_A, GPIO_IN);
    gpio_pull_up(but_A);
    
    // Inicialização do PWM para LEDs RGB
    uint pwm_wrap = 4096;  
    pwm_init_gpio(led_r, pwm_wrap);
    pwm_init_gpio(led_b, pwm_wrap);
    
    uint32_t last_print_time = 0;
    uint16_t adc_y, adc_x;
    uint8_t square_x = 60;
    uint8_t square_y = 28;
    uint32_t pwmr;
    uint32_t pwmb;
    
    // Configuração do barramento I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Inicialização do display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    
    // Configuração das interrupções para os botões
    gpio_set_irq_enabled_with_callback(but_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(but_joystick, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    while(1){
        // Leitura do joystick (ADC)
        adc_select_input(0);
        adc_y = adc_read();
        adc_select_input(1);
        adc_x = adc_read();
        
        bool joystick_released = is_joystick_in_rest_position(adc_x, adc_y);
        update_square_position(adc_x, adc_y, &square_x, &square_y, joystick_released);
        
        // Atualização da tela OLED
        ssd1306_fill(&ssd, false);
        if(joystick_press) {
            ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);
        } else {
            ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);
        }
        draw_square(&ssd, square_x, square_y, square_size, true);
        ssd1306_send_data(&ssd);
        
        // Controle do brilho dos LEDs com base na posição do joystick
        if(!A_press){
            pwmb = abs((adc_y - 2048) * pwm_wrap) / 2048;
            pwmr = abs((adc_x - 2048) * pwm_wrap) / 2048;
            pwm_set_gpio_level(led_b, pwmb);
            pwm_set_gpio_level(led_r, pwmr);
        } else {
            pwm_set_gpio_level(led_b, 0);
            pwm_set_gpio_level(led_r, 0);
        }
        
        // Impressão dos valores do ADC a cada segundo
        uint32_t current_time = to_ms_since_boot(get_absolute_time());  
        if (current_time - last_print_time >= 1000) {
            printf("ADC Y: %u\n", adc_y);
            printf("ADC X: %u\n", adc_x);
            last_print_time = current_time;
        }
        sleep_ms(10);
    }
}

// Manipulador de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events){
    if(to_us_since_boot(get_absolute_time()) - last_time > 200000){
        if(gpio == but_A){
            A_press = !A_press;
        } else {
            led_g_on = !led_g_on;
            gpio_put(led_g, led_g_on);
            joystick_press = !joystick_press;
        }
        last_time = to_us_since_boot(get_absolute_time());
    }
}
