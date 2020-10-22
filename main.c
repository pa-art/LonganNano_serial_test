#include "lcd/lcd.h"
#include "gd32v_pjt_include.h"
#include <stdio.h>
#include <string.h>
 
void init_uart0(void)
{   
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);
 
    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
 
    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
 
//    usart_interrupt_enable(USART0, USART_INT_RBNE);
}
 
int main(void) {
    uint16_t c, cnt;
//    u8 buffer[1024];
    u8 x = 0, y = 0;

    // LED output setting
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1|GPIO_PIN_2);

    // initialize USART0
    init_uart0();
    // initialize OLED
    Lcd_Init();
    // lights LEDs 
    LEDR(1); LEDG(1); LEDB(1);
    // clear OLED
    LCD_Clear(BLACK);
    // enable read buffer not empty interrupt flag of USART0
    usart_interrupt_enable(USART0, USART_INT_FLAG_RBNE);

    while (1) {
        // if read buffer is not empty
        if (usart_flag_get(USART0, USART_INT_FLAG_RBNE) == SET) {
            // while read buffer is not empty
            while (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) == SET) {
                // read character from USART0
                c = usart_data_receive(USART0);
                // put character to USART0 (echo back)
                _put_char((u8)c);

                // put character to OLED
                LCD_ShowChar(x, y, (u8)c, 1, WHITE);
                // cursor control
                x += 8;
                if (x > 18 * 8) {
                    x = 0;
                    y += 16;
                    if (y > 4 * 16) {
                        LCD_Clear(BLACK);
                        y = 0;
                    }
                }

            }
        // if read buffer is empty
        } else {
            // flip LEDs
            LEDR_TOG; delay_1ms(20);
            LEDG_TOG; delay_1ms(20);
            LEDB_TOG; delay_1ms(20); 
        }
    }
}

void USART0_IRQn_Handler(void) {
    
}

// put a character to USART0
int _put_char(int ch)
{
    usart_data_transmit(USART0, (uint8_t) ch );
    while ( usart_flag_get(USART0, USART_FLAG_TBE)== RESET){
    }
 
    return ch;
}
 
#include <stdarg.h>
void usart_printf(const char *fmt, ...) {
    char buf[100];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
 
    char *p = buf;
    while( *p != '\0' ) {
        _put_char(*p);
        p++;
    }
}