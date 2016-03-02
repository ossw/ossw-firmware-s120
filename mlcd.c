#include "mlcd.h"
#include "spi.h"
#include "ext_ram.h"
#include "nrf_gpio.h"
#include "common.h"
#include "board.h"
#include "app_timer.h"
#include <inttypes.h>
#include <string.h>
#include "fs.h"
#include "nordic_common.h"
#include "app_scheduler.h"

#define TEMP_BL_TIMEOUT_UNIT            APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)

#define MLCD_BL_OFF 			0x0
#define MLCD_BL_ON 				0x1
#define MLCD_BL_ON_TEMP 	0x10

#define MLCD_IS_LINE_CHANGED(line_no) (fb_line_changes[line_no>>3]>>(line_no&0x7) & 0x1)
#define MLCD_SET_LINE_CHANGED(line_no) (fb_line_changes[line_no>>3]|=1<<(line_no&0x7))
#define MLCD_CLEAR_LINE_CHANGED(line_no) (fb_line_changes[line_no>>3]&=~(1<<(line_no&0x7)))

static uint8_t fb_line_changes[MLCD_YRES/8];
static uint8_t vcom;
static uint8_t bl_mode = MLCD_BL_OFF;
static uint8_t temp_bl_timeout = 5;
static bool colors_inverted = false;
static bool toggle_colors = false;

static app_timer_id_t mlcd_bl_timer_id;

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

void mlcd_off_event(void * p_event_data, uint16_t event_size)
{
		mlcd_backlight_off();
}

void mlcd_bl_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
		if (bl_mode == MLCD_BL_ON_TEMP) {
				uint32_t err_code = app_sched_event_put(NULL, NULL, mlcd_off_event);
				APP_ERROR_CHECK(err_code);
		}
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


void mlcd_timers_init(void)
{
    uint32_t err_code;	 
    err_code = app_timer_create(&mlcd_bl_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                mlcd_bl_timeout_handler);
    APP_ERROR_CHECK(err_code);
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
	app_timer_stop(mlcd_bl_timer_id);
	bl_mode = MLCD_BL_OFF;
  nrf_gpio_pin_clear(LCD_BACKLIGHT);
}

void mlcd_backlight_on(void)
{
	app_timer_stop(mlcd_bl_timer_id);
	bl_mode = MLCD_BL_ON;
  nrf_gpio_pin_set(LCD_BACKLIGHT);
}

void mlcd_backlight_temp_on(void) {
	if (bl_mode == MLCD_BL_ON) {
			return;
	}
	app_timer_stop(mlcd_bl_timer_id);
	bl_mode = MLCD_BL_ON_TEMP;
  nrf_gpio_pin_set(LCD_BACKLIGHT);
	app_timer_start(mlcd_bl_timer_id, temp_bl_timeout * TEMP_BL_TIMEOUT_UNIT, NULL);
}

void mlcd_backlight_temp_extend(void) {
		if (bl_mode == MLCD_BL_ON_TEMP) {
				mlcd_backlight_temp_on();
		}
}

void mlcd_backlight_toggle(void)
{
	switch (bl_mode) {
			case MLCD_BL_OFF:
					mlcd_backlight_on();
					break;
			case MLCD_BL_ON:
					mlcd_backlight_off();
					break;
			case MLCD_BL_ON_TEMP:
					mlcd_backlight_on();
					break;
	}
}

uint32_t mlcd_temp_backlight_timeout(void) {
		return temp_bl_timeout;
}

void mlcd_set_temp_backlight_timeout(int32_t timeout) {
		if (timeout > 20) {
				timeout = 20;
		} else if (timeout < 1) {
				timeout = 1;
		}
		temp_bl_timeout = timeout;
}

void mlcd_switch_vcom() {
	  if (vcom == VCOM_LO) {
			  vcom = VCOM_HI;
		} else {
			  vcom = VCOM_LO;
		}
}

void mlcd_fb_invalidate_all() {
    memset(fb_line_changes, 0xFF, MLCD_YRES/8);
}

void mlcd_fb_clear() {
	  ext_ram_fill(0, 0x0, MLCD_LINE_BYTES * MLCD_YRES);
	  mlcd_fb_invalidate_all();
}


void mlcd_fb_flush(void) {
		mlcd_fb_flush_with_param(false);
}

void mlcd_fb_flush_with_param(bool force_colors) {
		if (toggle_colors) {
				colors_inverted = !colors_inverted;
				mlcd_fb_invalidate_all();
				toggle_colors = false;
		}
	
    uint8_t command = MLCD_WR | vcom;
    uint8_t dummy = 0;
	  uint8_t line_address;
	  uint16_t ext_ram_line_address = 0;
  
	  bool any_changes = false;
		for (int line_no=0; line_no< MLCD_YRES; line_no++ ){
				if (MLCD_IS_LINE_CHANGED(line_no)) {
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
  
    for (uint8_t line_no = 0; line_no < MLCD_YRES; line_no++) {
			
			  if (MLCD_IS_LINE_CHANGED(line_no)){
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
					  spi_master_rx_to_tx_no_cs(EXT_RAM_SPI, MLCD_SPI, MLCD_LINE_BYTES, colors_inverted && !force_colors);
						/* disable ext ram */
				    nrf_gpio_pin_set(EXT_RAM_SPI_SS);
					
            spi_master_tx_data_no_cs(MLCD_SPI, &dummy, 1);
						
						MLCD_CLEAR_LINE_CHANGED(line_no);
				}
			
				ext_ram_line_address += MLCD_LINE_BYTES;
    }
    spi_master_tx_data_no_cs(MLCD_SPI, &dummy, 1);

    /* disable slave (slave select active HIGH) */
    nrf_gpio_pin_clear(MLCD_SPI_SS);
}

bool is_mlcd_inverted() {
		return colors_inverted;
}

void mlcd_colors_toggle(void) {
		toggle_colors = true;
}

void mlcd_fb_draw_with_func(uint_fast8_t (*f)(uint_fast8_t, uint_fast8_t), uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height) {
	  x_pos = MLCD_XRES - x_pos - width;
	  uint_fast8_t start_bit_off = x_pos & 0x7;
	  uint_fast8_t first_byte_max_bit = width + start_bit_off;
	  if (first_byte_max_bit > 8) {
			  first_byte_max_bit = 8;
		}
    uint8_t line_size = (start_bit_off + width + 7) >> 3;
    uint8_t tmp_buff[MLCD_LINE_BYTES];
    uint16_t ext_ram_address = EXT_RAM_DATA_FRAME_BUFFER + (x_pos >> 3) + y_pos * MLCD_LINE_BYTES;
		
    uint8_t old_val = 0;
	  for (uint8_t y = 0; y < height; y++) {
			  uint8_t x = width-1;
			  uint_fast8_t width_left = width;
        uint8_t val = 0;
			
			  MLCD_SET_LINE_CHANGED((y_pos+y));
			
			  if (start_bit_off > 0 || width_left < 8 - start_bit_off) {
					  ext_ram_read_data(ext_ram_address, &old_val, 1);
							
						if ( start_bit_off > 0 ){
								val = old_val & (0xFF << (8 - start_bit_off));
						}
						
						if ( width_left < 8 - start_bit_off) {
								val |= old_val & (0xFF >> (width_left + start_bit_off));
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

void mlcd_fb_draw_bitmap(const uint8_t *bitmap, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t bitmap_width) {
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
    uint8_t tmp_buff[MLCD_LINE_BYTES];
    uint16_t ext_ram_address = EXT_RAM_DATA_FRAME_BUFFER + (x_pos >> 3) + y_pos * MLCD_LINE_BYTES;
		uint8_t byte_width = (bitmap_width+7)>>3;
    uint8_t old_val = 0;
	  for (uint8_t y = 0; y < height; y++) {
			  uint8_t x = width-1;
			  uint_fast8_t width_left = width;
        uint8_t val = 0;
			  uint_fast8_t byte_no = 0;
			
			  MLCD_SET_LINE_CHANGED((y_pos+y));
			
			  if (start_bit_off > 0 || width_left < 8 - start_bit_off) {
					  ext_ram_read_data(ext_ram_address, &old_val, 1);
							
						if ( start_bit_off > 0 ){
								val = old_val & (0xFF << (8 - start_bit_off));
						}
						
						if ( width_left < 8 - start_bit_off) {
								val |= old_val & (0xFF >> (width_left + start_bit_off));
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

struct mlcd_fb_draw_bitmap_from_file_data {
		uint_fast8_t start_bit_off;
	  uint_fast8_t first_byte_max_bit;
    uint8_t line_size;
    uint16_t ext_ram_address;
		uint8_t byte_width;
		uint8_t y;
		uint8_t width;
};

static void mlcd_fb_draw_bitmap_from_file_handle(struct mlcd_fb_draw_bitmap_from_file_data* data, uint8_t* chank_buf) {
				uint8_t tmp_buff[MLCD_LINE_BYTES];
				uint8_t old_val = 0;
			  uint8_t x = data->width-1;
			  uint_fast8_t width_left = data->width;
        uint8_t val = 0;
			  uint_fast8_t byte_no = 0;
			
			  MLCD_SET_LINE_CHANGED(data->y);
			
			  if (data->start_bit_off > 0 || width_left < 8 - data->start_bit_off) {
					  ext_ram_read_data(data->ext_ram_address, &old_val, 1);
							
						if ( data->start_bit_off > 0 ){
								val = old_val & (0xFF << (8 - data->start_bit_off));
						}
						
						if ( width_left < 8 - data->start_bit_off) {
								val |= old_val & (0xFF >> (width_left + data->start_bit_off));
						}
			  }
					  
				for(uint_fast8_t bit=data->start_bit_off; bit<data->first_byte_max_bit; bit++){
            val |= (((chank_buf[x >> 3] >> (~x&0x7)) & 0x1) << (7-bit));
			  	  x--;
        }
				tmp_buff[byte_no++] = val;
				width_left -= data->first_byte_max_bit - data->start_bit_off;
				
				while(width_left > 0) {
				    if ( width_left < 8) {
					      ext_ram_read_data(data->ext_ram_address + byte_no, &val, 1);
				    	  val &= (0xFF >> width_left);
							
						    for(uint_fast8_t bit=0; bit<width_left; bit++){
                    val |= (((chank_buf[x >> 3] >> (~x&0x7)) & 0x1) << (7-bit));
			  	          x--;
                }
					      width_left = 0;
				    } else {
					      val = 0;
					    	for(uint_fast8_t bit=0; bit<8; bit++){
                    val |= (((chank_buf[x >> 3] >> (~x&0x7)) & 0x1) << (7-bit));
			  	          x--;
                }
								width_left -= 8;
						}
				    tmp_buff[byte_no++] = val;
				}
				ext_ram_write_data(data->ext_ram_address, tmp_buff, data->line_size);
				data->ext_ram_address += MLCD_LINE_BYTES;
				data->y++;
}

void mlcd_fb_draw_bitmap_from_file(spiffs_file file, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t bitmap_width) {
		struct mlcd_fb_draw_bitmap_from_file_data data;
		if (x_pos + width > MLCD_XRES) {
			  if (x_pos >= MLCD_XRES) {
					  return;
				}
			  width = MLCD_XRES - x_pos;
		}
		x_pos = MLCD_XRES - x_pos - width;
	  data.start_bit_off = x_pos & 0x7;
	  data.first_byte_max_bit = width + data.start_bit_off;
	  if (data.first_byte_max_bit > 8) {
			  data.first_byte_max_bit = 8;
		}
    data.line_size = (data.start_bit_off + width + 7) >> 3;
    data.ext_ram_address = EXT_RAM_DATA_FRAME_BUFFER + (x_pos >> 3) + y_pos * MLCD_LINE_BYTES;
		data.byte_width = (bitmap_width+7)>>3;
		uint8_t bitmap[MLCD_LINE_BYTES];
		data.y = y_pos;
		data.width = width;
		
		SPIFFS_read_notify(&fs, file, bitmap, height, data.byte_width, (void (*)(void*, void*))&mlcd_fb_draw_bitmap_from_file_handle, &data);
}

void mlcd_set_line_changed(uint_fast8_t y) {
		MLCD_SET_LINE_CHANGED(y);
}
