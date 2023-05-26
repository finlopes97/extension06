/***
 * Ex E6.0
 * 
 * Your task is to write code which handles a sequence of input characters
 * from the UART interface, and respond with the output specified below.
 * It is strongly recommended that you design a state machine to complete
 * this task, and practice drawing a state machine diagram before you
 * begin coding.
 * 
 * On receipt of the character sequence: 
 *   "foo" your programme should print '0' to the UART interface.
 *   "bar" your programme should print '1' to the UART interface.
 *   "foobar" your program should not print either '0' nor '1' as
 *   specified above, but should instead print a linefeed ('\n') 
 *   character.
 * 
 * Your solution should use a baud rate of 9600, and 8N1 frame format.
 * Your solution MUST NOT use qutyio.o or qutyserial.o.
 * 
 * Examples:
 * 
 *   Input:  
 *     ...foo.bar.foo.barfoobarfood
 *   Output: 
 *     0101
 *     0
 * 
 *   Input:
 *     barsfoosbarforbarfoobarrforfoobarfoobarfood
 *   Output:
 *     1011
 *     
 *     
 *     0
 */
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include "header_2_electric_boogaloo.h"

typedef enum {
    START,
    PRE_FOO,
    PRE_BAR,
    FOO_COMP,
    BAR_COMP,
    CONT,
    COMP,
    CLEAN
} foobar_states;

void uart_init(void) {
    PORTB.DIRSET = PIN2_bm; // set TX pin as output
    USART0.BAUD = 1389; // set baud rate to 9600
    // Set the frame format to 8N1 - 8 data bits, no parity bit and 1 stop bit
    USART0.CTRLC = (USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc);
    USART0.CTRLB = (USART_RXEN_bm | USART_TXEN_bm); // enable RX and TX
}

uint8_t uart_getc(void) {
    while (!(USART0.STATUS & USART_RXCIF_bm)); // Wait for data
    return USART0.RXDATAL;
}

void uart_putc(uint8_t c) {
    while (!(USART0.STATUS & USART_DREIF_bm)); // Wait for TXDATA empty
    USART0.TXDATAL = c;
}

int main(void) {
    uart_init();
    foobar_states state = START;
    volatile uint8_t cur_char = 0;
    volatile uint8_t cont = 0;
    volatile uint8_t payload = 0;

    while (1) {
        switch (state) {
            case START:
                cur_char = uart_getc();
                if (cur_char == 'f') { state = PRE_FOO; }
                else if (cur_char == 'b') { state = PRE_BAR; }
                else { ; }
                break;

            case PRE_FOO:
                cur_char = uart_getc();
                if (cur_char == 'o') { state = FOO_COMP; }
                else if (cur_char == 'f') { ; }
                else { state = CLEAN; }
                break;

            case FOO_COMP:
                cur_char = uart_getc();
                if (cur_char == 'o') { state = CONT; }
                else { state = CLEAN; }
                break;

            case PRE_BAR:
                cur_char = uart_getc();
                if (cur_char == 'a') { state = BAR_COMP; }
                else if (cur_char != 'a' && cont == 1) {
                    payload = '0';
                    state = COMP;
                } else if (cur_char == 'b') { cont = 0; }
                else { state = CLEAN; }
                break;

            case BAR_COMP:
                cur_char = uart_getc();
                if (cur_char == 'r') {
                    if (cont == 1) { payload = '\n'; }
                    else if (cont == 0) { payload = '1'; }
                    state = COMP;
                } else if (cur_char != 'r' && cont == 1) {
                    payload = '0';
                    state = COMP;
                } else { state = CLEAN; }
                break;

            case CONT:
                cur_char = uart_getc();
                if (cur_char == 'b') {
                    state = PRE_BAR;
                    cont = 1;
                } else {
                    payload = '0';
                    state = COMP;
                }
                break;

            case COMP:
                uart_putc(payload);
                state = CLEAN;
                break;

            case CLEAN:
                cur_char = 0;
                payload = 0;
                cont = 0;
                state = START;
                break;

            default:
                state = START;
                break;
        }
    }
} // end main()
