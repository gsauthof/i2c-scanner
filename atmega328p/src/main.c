// Scan for I2C devices on a ATmega328p 8 MHz Arduino Pro Mini
//
// 2022-08-20 Georg Sauthoff <mail@gms.tf>


#include <avr/io.h>
#include <avr/sfr_defs.h>

#include <util/delay.h>
#include <util/twi.h>

#include <stdio.h>


// copied and modified from: https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n')
        uart_putchar('\r', stream);
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}

static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);



static void setup_uart()
{
    // in case UART was put into power-reduction mode ...
    PRR &= ~ _BV(PRUSART0);

    // configure speed
#define BAUD 38400
    // NB: <- max rate with 8MHz ATmega328p
#include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    // USART Control and Status Register A (Port 0)
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~ _BV(U2X0);
#endif
#undef BAUD

    // USART Control and Status Register B:
    // - Enable transmitter only
    UCSR0B = _BV(TXEN0);
    // 8N1
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}


// at 8 Mhz
static void i2c_set_100kHz()
{
    TWBR = 32;
}
static void i2c_set_400kHz()
{
    TWBR = 2;
}

// NB: clearing TWINT starts next transmission
//     TWI == I2C
//     TWCR = TWI Control Register
//     TWDR = TWI Data Register

// start master transmission
static void i2c_start()
{
    // clear TWINT flag, send START, enable TWI unit
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    loop_until_bit_is_set(TWCR, TWINT);
    // NB: TWSTA must be cleared explicitly in the next operation
}
static void i2c_stop()
{
    // clear TWINT flag, send STOP, enable TWI unit
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
    loop_until_bit_is_clear(TWCR, TWSTO);
    // NB: TWSTO is cleared automatically
    // NB: TWINT is NOT set after STOP transmission ...
}
// rw: TW_READ (1) or TW_WRITE (0)
static void i2c_set_address(uint8_t addr, uint8_t rw)
{
    TWDR = (addr << 1) | rw;
    // clear TWINT flag, send STOP, enable TWI unit
    TWCR = _BV(TWINT) | _BV(TWEN);
    loop_until_bit_is_set(TWCR, TWINT);
}

static void setup()
{
    setup_uart();
    stdout = &uart_stdout;
    i2c_set_100kHz();

    // uncomment if your I2C modules don't come with pull-ups ...
    // DDRC &= ~ _BV(DDC4); PORTC |= _BV(PORTC4);
    // DDRC &= ~ _BV(DDC5); PORTC |= _BV(PORTC5);
}

static void probe_address(uint8_t i)
{
    i2c_start();
    i2c_set_address(i, TW_WRITE);
    if ((TWSR & TW_STATUS_MASK) == TW_MT_SLA_ACK)
         printf("Found device on address: 0x%" PRIx8 " (%" PRIu8 ")\n", i, i);
    i2c_stop();
}

static void scan()
{
    // skipping reserved addresses
    // cf. https://en.wikipedia.org/wiki/I%C2%B2C#Reserved_addresses_in_7-bit_address_space
    for (uint32_t i = 8; i<128; ++i)
        probe_address(i);
}

int main()
{
    setup();

    for (;;) {
        printf("Scanning I2C bus at 100 kHz ...\n");
        scan();

        printf("Scanning I2C bus at 400 kHz ...\n");
        i2c_set_400kHz();
        scan();

        printf("done\n\n");

        i2c_set_100kHz();

        for (uint16_t i = 0; i < 30 * 30; ++i)
            _delay_ms(32);
    }

    return 0;
}
