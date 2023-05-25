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
    START, // Initial state, user can enter 'f' or 'b', save to var x
    ESCAPE, // Escape state, user can enter 'o' or 'b', check x and goto PRE_FOO or PRE_BAR
    PRE_FOO, // User can enter 'o', goto FOO_COMP
    PRE_BAR, // User can enter 'a', goto BAR_COMP
    FOO_COMP, // User can enter 'b' to go to ESCAPE, set CONT == 1, any other character print '0', goto COMP
    BAR_COMP, // User can enter any character print, if CONT == 0 print '1', goto NEWLINE
    NEWLINE, // If CONT == 1, print line feed '\n', set CONT == 0, goto START
    COMP // Clean exit, goto START
} state;

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
    state state_of_denmark = START;
    volatile uint8_t x = 0;
    volatile uint8_t CONT = 0;

    while (1) {
        switch (state_of_denmark) {
            case START:
                x = uart_getc();
                state_of_denmark = ESCAPE;
                break;

            case ESCAPE:
                if (x == 'f' && uart_getc() == 'o') {
                    state_of_denmark = PRE_FOO;
                } else if (x == 'b' && uart_getc() == 'a') {
                    state_of_denmark = PRE_BAR;
                } else { state_of_denmark = START; }
                break;

            case PRE_FOO:
                if (uart_getc() == 'o') {
                    state_of_denmark = FOO_COMP;
                }
                break;

            case PRE_BAR:
                if (uart_getc() == 'r') {
                    state_of_denmark = BAR_COMP;
                }
                break;

            case FOO_COMP:
                if (uart_getc() == 'b') {
                    x = 'b';
                    state_of_denmark = ESCAPE;
                    CONT = 1;
                } else {
                    uart_putc('0');
                    state_of_denmark = COMP;
                }
                break;

            case BAR_COMP:
                if (CONT == 1) {
                    state_of_denmark = NEWLINE;
                } else if (CONT == 0 && uart_getc()) {
                    uart_putc('1');
                    state_of_denmark = COMP;
                }
                break;

            case NEWLINE:
                uart_putc('\n');
                CONT = 0;
                state_of_denmark = START;
                break;

            case COMP:
                state_of_denmark = START;
                break;

            default:
                state_of_denmark = START;
                break;
        }
    }

} // end main()
