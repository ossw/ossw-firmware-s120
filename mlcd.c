#include "mlcd.h"
#include "spi.h"
#include "ext_ram.h"
#include "nrf_gpio.h"
#include "common.h"
#include "board.h"
#include <inttypes.h>

//static uint8_t fb[MLCD_LINE_BYTES * MLCD_YRES];
static bool fb_line_changes[MLCD_YRES];
static uint8_t vcom;

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
	  vcom = VCOM_LO;
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

void mlcd_switch_vcom() {
	  if (vcom == VCOM_LO) {
			  vcom = VCOM_HI;
		} else {
			  vcom = VCOM_LO;
		}
}

void mlcd_fb_clear() {
	  ext_ram_fill(0 | vcom, 0x0, MLCD_LINE_BYTES * MLCD_YRES);
	  for (int line_no=0; line_no< MLCD_YRES; line_no++ ){
				fb_line_changes[line_no] = true;
		}
}

void mlcd_fb_flush () {
    uint8_t command = MLCD_WR | vcom;
    uint8_t dummy = 0;
	  uint8_t line_address;
	  uint16_t ext_ram_line_address = 0;
  
	  bool any_changes = false;
		for (int line_no=0; line_no< MLCD_YRES; line_no++ ){
				if(fb_line_changes[line_no]) {
					  any_changes = true;
					  break;
				}
		}
		
		if (!any_changes) {
			  // nothing to update
			  return;
		}
	
    /* enable slave (slave select active HIGH) */
    nrf_gpio_pin_set(MLCD_SPI_SS);
  
    spi_master_tx_data_no_cs(MLCD_SPI, &command, 1);
  
    for(uint8_t line_no = 0; line_no < MLCD_YRES; line_no++) {
			
			  if (fb_line_changes[line_no]){
					  // line changed, send line update
            line_address = bit_reverse(MLCD_YRES - line_no);
            spi_master_tx_data_no_cs(MLCD_SPI, &line_address, 1);
					
						/* enable ext ram */
	          uint8_t command[] = {EXT_RAM_READ_COMMAND, 0xFF, 0xFF};
            command[1] = ext_ram_line_address >> 8 & 0xFF;
            command[2] = ext_ram_line_address & 0xFF;
						
						nrf_gpio_pin_clear(EXT_RAM_SPI_SS);
  					spi_master_tx_data_no_cs(EXT_RAM_SPI, command, 3);
					  
						/* send response from ram to mlcd */
					  spi_master_rx_to_tx_no_cs(EXT_RAM_SPI, MLCD_SPI, MLCD_LINE_BYTES);
						/* disable ext ram */
				    nrf_gpio_pin_set(EXT_RAM_SPI_SS);
					
            spi_master_tx_data_no_cs(MLCD_SPI, &dummy, 1);
						
						fb_line_changes[line_no] = false;
				}
			
				ext_ram_line_address += MLCD_LINE_BYTES;
    }
    spi_master_tx_data_no_cs(MLCD_SPI, &dummy, 1);

    /* disable slave (slave select active HIGH) */
    nrf_gpio_pin_clear(MLCD_SPI_SS);
}

void mlcd_fb_draw_with_func(uint_fast8_t (*f)(uint_fast8_t, uint_fast8_t), uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height) {
	  x_pos = MLCD_XRES - x_pos - width;
	  uint_fast8_t start_bit_off = x_pos & 0x7;
	  uint_fast8_t first_byte_max_bit = width + start_bit_off;
	  if (first_byte_max_bit > 8) {
			  first_byte_max_bit = 8;
		}
    uint8_t line_size = (start_bit_off + width + 7) >> 3;
    uint8_t tmp_buff[line_size];
    uint16_t ext_ram_address = EXT_RAM_DATA_FB + (x_pos >> 3) + y_pos * MLCD_LINE_BYTES;
		
    uint8_t old_val = 0;
	  for (uint8_t y = 0; y < height; y++) {
			  uint8_t x = width-1;
			  uint_fast8_t width_left = width;
        uint8_t val = 0;
			
			  fb_line_changes[y_pos+y] = true;
			
			  if (start_bit_off > 0 || width_left < 8 - start_bit_off) {
					  ext_ram_read_data(ext_ram_address, &old_val, 1);
							
						if ( start_bit_off > 0 ){
								val = old_val & (0xFF << 8 - start_bit_off);
						}
						
						if ( width_left < 8 - start_bit_off) {
								val |= old_val & (0xFF >> width_left + start_bit_off);
						}
			  }
					  
				for(uint_fast8_t bit=start_bit_off; bit<first_byte_max_bit; bit++){
            val |= ((*f)(x, y) << (7-bit));
			  	  x--;
        }
				tmp_buff[0] = val;
				width_left -= first_byte_max_bit - start_bit_off;
			  uint_fast8_t byte_no = 1;
				
				while(width_left > 0) {
				    if ( width_left < 8) {
					      ext_ram_read_data(ext_ram_address + byte_no, &val, 1);
				    	  val &= (0xFF >> width_left);
							
						    for(uint_fast8_t bit=0; bit<width_left; bit++){
                    val |= ((*f)(x, y) << (7-bit));
			  	          x--;
                }
					      width_left = 0;
				    } else {
					      val = 0;
					    	for(uint_fast8_t bit=0; bit<8; bit++){
                    val |= ((*f)(x, y) << (7-bit));
			  	          x--;
                }
								width_left -= 8;
						}
				    tmp_buff[byte_no++] = val;
				}
				ext_ram_write_data(ext_ram_address, tmp_buff, line_size);
				ext_ram_address += MLCD_LINE_BYTES;
	  }
}

void mlcd_fb_draw_bitmap(const uint8_t *bitmap, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height) {
	  if (x_pos + width > MLCD_XRES) {
			  if (x_pos >= MLCD_XRES) {
					  return;
				}
			  width = MLCD_XRES - x_pos;
		}
		x_pos = MLCD_XRES - x_pos - width;
	  uint_fast8_t start_bit_off = x_pos & 0x7;
	  uint_fast8_t first_byte_max_bit = width + start_bit_off;
	  if (first_byte_max_bit > 8) {
			  first_byte_max_bit = 8;
		}
    uint8_t line_size = (start_bit_off + width + 7) >> 3;
    uint8_t tmp_buff[line_size];
    uint16_t ext_ram_address = EXT_RAM_DATA_FB + (x_pos >> 3) + y_pos * MLCD_LINE_BYTES;
		uint8_t byte_width = (width+7)>>3;
    uint8_t old_val = 0;
	  for (uint8_t y = 0; y < height; y++) {
			  uint8_t x = width-1;
			  uint_fast8_t width_left = width;
        uint8_t val = 0;
			  uint_fast8_t byte_no = 0;
			
			  fb_line_changes[y_pos+y] = true;
			
			  if (start_bit_off > 0 || width_left < 8 - start_bit_off) {
					  ext_ram_read_data(ext_ram_address, &old_val, 1);
							
						if ( start_bit_off > 0 ){
								val = old_val & (0xFF << 8 - start_bit_off);
						}
						
						if ( width_left < 8 - start_bit_off) {
								val |= old_val & (0xFF >> width_left + start_bit_off);
						}
			  }
					  
				for(uint_fast8_t bit=start_bit_off; bit<first_byte_max_bit; bit++){
            val |= (((bitmap[x >> 3] >> (~x&0x7)) & 0x1) << (7-bit));
			  	  x--;
        }
				tmp_buff[byte_no++] = val;
				width_left -= first_byte_max_bit - start_bit_off;
				
				while(width_left > 0) {
				    if ( width_left < 8) {
					      ext_ram_read_data(ext_ram_address + byte_no, &val, 1);
				    	  val &= (0xFF >> width_left);
							
						    for(uint_fast8_t bit=0; bit<width_left; bit++){
                    val |= (((bitmap[x >> 3] >> (~x&0x7)) & 0x1) << (7-bit));
			  	          x--;
                }
					      width_left = 0;
				    } else {
					      val = 0;
					    	for(uint_fast8_t bit=0; bit<8; bit++){
                    val |= (((bitmap[x >> 3] >> (~x&0x7)) & 0x1) << (7-bit));
			  	          x--;
                }
								width_left -= 8;
						}
				    tmp_buff[byte_no++] = val;
				}
				ext_ram_write_data(ext_ram_address, tmp_buff, line_size);
				ext_ram_address += MLCD_LINE_BYTES;
				bitmap += byte_width;
	  }
}
