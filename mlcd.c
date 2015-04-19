
#include "mlcd.h"
#include "spi.h"
#include "nrf_gpio.h"
#include "common.h"

static uint8_t bit_reverse(uint8_t byte) {
    #if (__CORTEX_M >= 0x03)
        return (uint8_t)(__RBIT(byte) >> 24);
    #else /* #if (__CORTEX_M < 0x03) */
        uint8_t rev8 = 0;
        for (unsigned i = 0; i < 8; i++) {
            if (byte & (1 << i)) {
                rev8 |= 1 << (7 - i);
            }
        }
        return rev8;
    #endif /* #if (__CORTEX_M >= 0x03) */
}

void mlcd_init(void)
{
    nrf_gpio_cfg_output(LCD_ENABLE);
    nrf_gpio_cfg_output(LCD_BACKLIGHT);
    nrf_gpio_cfg_output(LCD_VOLTAGE_REG);
    nrf_gpio_pin_clear(LCD_ENABLE);
    nrf_gpio_pin_clear(LCD_BACKLIGHT);
    nrf_gpio_pin_clear(LCD_VOLTAGE_REG);
}

void mlcd_display_off(void)
{
  nrf_gpio_pin_clear(LCD_ENABLE);
}

void mlcd_display_on(void)
{
  nrf_gpio_pin_set(LCD_ENABLE);
}

void mlcd_power_off(void)
{
  nrf_gpio_pin_clear(LCD_VOLTAGE_REG);
}

void mlcd_power_on(void)
{
  nrf_gpio_pin_set(LCD_VOLTAGE_REG);
}

void mlcd_backlight_off(void)
{
  nrf_gpio_pin_clear(LCD_BACKLIGHT);
}

void mlcd_backlight_on(void)
{
  nrf_gpio_pin_set(LCD_BACKLIGHT);
}

void mlcd_set_lines_with_func(uint8_t (*f)(uint8_t, uint8_t), uint8_t first_line, uint8_t line_number) {
    NRF_SPI_Type *spi_base = (NRF_SPI_Type *)MLCD_SPI;

    uint8_t command = MLCD_WR;
    uint8_t dummy = 0;
    uint8_t line_buffer[MLCD_LINE_BYTES+2];
    uint8_t max_line = first_line + line_number;
  
    /* enable slave (slave select active HIGH) */
    nrf_gpio_pin_set(MLCD_SPI_SS);
  
    spi_master_tx_data_no_cs(spi_base, &command, 1);
  
    line_buffer[MLCD_LINE_BYTES+1] = 0;
    for(uint8_t line = first_line; line < max_line; line++) {
        line_buffer[0] = bit_reverse(line+1);
      
        for(uint8_t i=0; i < MLCD_LINE_BYTES; i++) {
            uint8_t val = 0;
            for(uint8_t bit=0; bit<8; bit++){
                val |= (*f)(i*8+bit, line) << (7-bit);
            }
            line_buffer[i+1] = val;
        }
        spi_master_tx_data_no_cs(spi_base, line_buffer, MLCD_LINE_BYTES+2);
    }
    spi_master_tx_data_no_cs(spi_base, &dummy, 1);

    /* disable slave (slave select active HIGH) */
    nrf_gpio_pin_clear(MLCD_SPI_SS);
}
  
void mlcd_set_screen_with_func(uint8_t (*f)(uint8_t, uint8_t)) {
    mlcd_set_lines_with_func(f, 0, MLCD_YRES);
}
