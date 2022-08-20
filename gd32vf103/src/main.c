// Scan for I2C devices on an Longan Nano
//
// 2022-08-20 Georg Sauthoff <mail@gms.tf>


#include <gd32vf103_i2c.h>
#include <gd32vf103_gpio.h>
#include <gd32vf103_rcu.h>

#include <inttypes.h>
#include <stdint.h>
// writes to UART, 115200,8N1, by default
#include <stdio.h>



static void probe_address(uint32_t i)
{
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY))
        ;
    i2c_start_on_bus(I2C0);
    while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND))
        ;
    // it's cleared by getting the flag, ie. reading I2C_STAT0
    i2c_master_addressing(I2C0, i << 1, I2C_TRANSMITTER);
    uint32_t k = 0;
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        if (i2c_flag_get(I2C0, I2C_FLAG_AERR)) {
            i2c_flag_clear(I2C0, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C0);
            return;
        }
        // in case no bus is connected
        if (k++ > 1000 * 1000) {
            i2c_stop_on_bus(I2C0);
            return;
        }
    }
    printf("Found device on address: 0x%" PRIx32 " (%" PRIu32 ")\n", i, i);
    // NB: it's cleared by reading I2C_STAT0 _and_ I2C_STAT1
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    i2c_stop_on_bus(I2C0);

    // wait for stop being sent
    while (I2C_CTL0(I2C0) & I2C_CTL0_STOP)
        ;
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
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_I2C0);
    // yes, I2C is open-drain
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_6 | GPIO_PIN_7) ;
    i2c_deinit(I2C0);
    i2c_clock_config(I2C0, 100000, I2C_DTCY_2);
    // NB: we don't call i2c_mode_addr_config()
    //     because we are only in master transmit mode
    i2c_enable(I2C0);

    for (;;) {
        printf("Scanning I2C bus at 100 kHz ...\n");
        scan();

        printf("Scanning I2C bus at 400 kHz ...\n");
        i2c_clock_config(I2C0, 400000, I2C_DTCY_2);
        scan();

        printf("done\n\n");

        i2c_clock_config(I2C0, 100000, I2C_DTCY_2);
        delay_1ms(30 * 1000);
    }

    return 0;
}
