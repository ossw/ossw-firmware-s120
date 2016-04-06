#include "mlcd_draw.h"
#include "mlcd.h"
#include "utf8.h"
#include "graph.h"
#include "fonts/font.h"
#include "fonts/small_regular.h"
#include "fonts/small_bold.h"
#include "fonts/normal_regular.h"
#include "fonts/normal_bold.h"
#include "fonts/big_regular.h"
#include "fonts/option_normal.h"
#include "fonts/option_big.h"

static const uint8_t digits[] = {0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x7, 0x7F, 0x6F};

static uint8_t draw_width;
static uint8_t draw_height;
static uint8_t draw_thickness;
static uint8_t draw_param;

static uint_fast8_t draw_digit_func(uint_fast8_t x, uint_fast8_t y) {
		uint8_t mask = digits[draw_param];
		
		if(mask & 0x1) {
				if (y < draw_thickness) {
						return 1;
				}	
		}
		if(mask & 0x2) {
				if(x >= draw_width-draw_thickness && y < draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x4) {
				if(x >= draw_width-draw_thickness && y >= draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x8) {
				if(y >= draw_height - draw_thickness) {
						return 1;
				}
		}
		if(mask & 0x10) {
				if(x < draw_thickness && y >= draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x20) {
				if(x < draw_thickness && y < draw_height/2) {
						return 1;
				}
		}
		if(mask & 0x40) {
				if(y >= (draw_height - draw_thickness)/2 && y < (draw_height + draw_thickness)/2) {
						return 1;
				}
		}
		return 0;
}

static uint_fast8_t draw_horizontal_progress_func(uint_fast8_t x, uint_fast8_t y) {
	  return x < draw_param ? 1 : 0;
}

static uint_fast8_t draw_vertical_progress_func(uint_fast8_t x, uint_fast8_t y) {
	  return y >= draw_param ? 1 : 0;
}

static uint_fast8_t clear_rect_func(uint_fast8_t x, uint_fast8_t y) {
	  return 0;
}

static uint_fast8_t draw_rect_func(uint_fast8_t x, uint_fast8_t y) {
	  return 1;
}

static uint_fast8_t draw_rect_border_func(uint_fast8_t x, uint_fast8_t y) {
  	return (x < draw_thickness || y < draw_thickness || x >= draw_width - draw_thickness || y >= draw_height - draw_thickness) ? 1 : 0;
}

static uint_fast8_t draw_arrow_up_func(uint_fast8_t x, uint_fast8_t y) {
	  return (
             (  (y < draw_height * x * 2 / draw_width) && (y < draw_height * (draw_width-x) * 2 / draw_width))
                &&
                (  (y + draw_thickness >= draw_height * x * 2 / draw_width) || (y + draw_thickness >= draw_height * (draw_width-x) * 2 / draw_width))
            
          ) ? 1 : 0;
}

static uint_fast8_t draw_arrow_down_func(uint_fast8_t x, uint_fast8_t y) {
	y = draw_height - y - 1;
	return (
             (  (y < draw_height * x * 2 / draw_width) && (y < draw_height * (draw_width-x) * 2 / draw_width))
                &&
                (  (y + draw_thickness >= draw_height * x * 2 / draw_width) || (y + draw_thickness >= draw_height * (draw_width-x) * 2 / draw_width))
            
          ) ? 1 : 0;
}

void mlcd_draw_digit(uint_fast8_t digit, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  draw_param = digit;
	  mlcd_fb_draw_with_func(draw_digit_func, x_pos, y_pos, width, height);
}

void mlcd_draw_simple_progress(uint_fast8_t value, uint_fast8_t max, uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, bool horizontal) {
		draw_width = width;
	  draw_height = height;
		draw_param = horizontal?(value*width/max):height - (value*height/max);
		mlcd_fb_draw_with_func(horizontal?draw_horizontal_progress_func:draw_vertical_progress_func, x_pos, y_pos, width, height);
}

void mlcd_clear_rect(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height) {
	  draw_width = width;
	  draw_height = height;
	  mlcd_fb_draw_with_func(clear_rect_func, x_pos, y_pos, width, height);
}

void mlcd_draw_rect(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height) {
	  draw_width = width;
	  draw_height = height;
	  mlcd_fb_draw_with_func(draw_rect_func, x_pos, y_pos, width, height);
}

void mlcd_draw_rect_border(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  mlcd_fb_draw_with_func(draw_rect_border_func, x_pos, y_pos, width, height);
}

void mlcd_draw_arrow_down(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  mlcd_fb_draw_with_func(draw_arrow_up_func, x_pos, y_pos, width, height);
}

void mlcd_draw_arrow_up(uint_fast8_t x_pos, uint_fast8_t y_pos, uint_fast8_t width, uint_fast8_t height, uint_fast8_t thickness) {
	  draw_width = width;
	  draw_height = height;
	  draw_thickness = thickness;
	  mlcd_fb_draw_with_func(draw_arrow_down_func, x_pos, y_pos, width, height);
}

static const FONT_CHAR_INFO* resolve_char_info(uint32_t c, const FONT_INFO* font) {
   if ((c < font->startChar) || (c > font->endChar)) 
      return NULL; 

	 const FONT_CHAR_INFO_LOOKUP *matchingTable = 0;
	 for (int t = 0; t < font->lookupTableSize; t++) {
	    const FONT_CHAR_INFO_LOOKUP *table = &(font->charInfoLookup[t]);
		  if (c >= table->startChar && c <= table->endChar) {
				  matchingTable = table;
			}
	 }
	 if (!matchingTable) {
		   return NULL;
	 }
   uint32_t charIndex = c - matchingTable->startChar;  //Character index 
   const FONT_CHAR_INFO *charInfo = matchingTable->charInfo;          //Point to start of descriptors 
   charInfo += charIndex;              //Point to current char info 
	 return charInfo;
}

static uint_fast8_t mlcd_draw_char(uint32_t c, uint_fast8_t x, uint_fast8_t y, uint_fast8_t width, uint_fast8_t height, const FONT_INFO* font){
	 if (c == ' ') {
	     return font->spaceWidth;
	 }
	
	 const FONT_CHAR_INFO *charInfo = resolve_char_info(c, font);
	 
	 if (charInfo == NULL) {
		   return font->spaceWidth;
	 }

   const uint8_t *bitmapPointer = font->data;                   //Point to start of bitmaps 
   bitmapPointer += charInfo->offset;            //Point to start of c bitmap 

	 uint8_t char_width = charInfo->width > width ? width : charInfo->width;
	 uint8_t char_height = font->height > height ? height : font->height;
	 mlcd_fb_draw_bitmap(bitmapPointer, x, y, char_width, char_height, charInfo->width);
   
   return char_width; 
}

const FONT_INFO* mlcd_resolve_font(uint_fast8_t font_type) {
	 switch (font_type & 0x1F) {
		 case FONT_SMALL_REGULAR:
			 return &smallRegularFontInfo;
		 case FONT_SMALL_BOLD:
			 return &smallBoldFontInfo;
		 case FONT_NORMAL_REGULAR:
			 return &normalRegularFontInfo;
		 case FONT_NORMAL_BOLD:
			 return &normalBoldFontInfo;
		 case FONT_BIG_REGULAR:
			 return &bigRegularFontInfo;
		 case FONT_OPTION_NORMAL:
			 return &optionNormalFontInfo;
		 case FONT_OPTION_BIG:
			 return &optionBigFontInfo;
	 }
	 return &normalRegularFontInfo;
}

static uint_fast8_t calc_char_width(uint32_t c, const FONT_INFO* font) {
		if (c == ' ') {
	      return font->spaceWidth;
		}
		const FONT_CHAR_INFO *charInfo = resolve_char_info(c, font);
		uint8_t width = 0;
		if (charInfo == NULL) {
				return font->spaceWidth;
		}	
		width += charInfo->width;	
		return width;
}

static inline bool is_new_line(int c) {
		return c == 10 || c == 13 || c == 11;
}

static inline bool is_whitespace(int c) {
		return c == ' ' || c == '\t' || is_new_line(c);
}

static uint_fast8_t calc_text_width(const char *text, int *ptr, uint_fast8_t font_type, bool split_word, uint8_t max_width) {
	  uint32_t c;
	  const FONT_INFO* font = mlcd_resolve_font(font_type);
	  uint_fast8_t width = 0;
		uint_fast8_t last_non_space = 0;
		uint_fast8_t last_word_end = 0;
	  while ((c = u8_nextchar(text, ptr)) > 0) {
				if (width > 0) {
						width += font->charDist;
				} else if (is_whitespace(c)) {
						continue;
				}
				if (is_new_line(c)){ 
						break;
				}
				width += calc_char_width(c, font);
				
				if (width > max_width) {
						width = (split_word || last_word_end == 0)? last_non_space : last_word_end;
						break;
				}
				
				if (is_whitespace(c)) {
						last_word_end = last_non_space;
				} else {
						last_non_space = width;
				}
		}
		return width > max_width ? max_width : width;
}

uint_fast8_t mlcd_calc_text_height(const char *text, uint_fast8_t start_x, uint_fast8_t start_y, uint_fast8_t width, uint_fast8_t height, uint_fast8_t font_type, uint8_t font_alignment) {
	  int ptr = 0;
		int prev_ptr = 0;
	  uint32_t c;
		
		uint8_t x = start_x;
		uint8_t y = start_y;
	  if (width == NULL) {
			  width = MLCD_XRES - x;
		}
	  if (height == NULL) {
			  height = MLCD_YRES - y;
		}
	  const FONT_INFO* font = mlcd_resolve_font(font_type);
	  bool multiline = font_alignment & MULTILINE;
	  bool split_word = font_alignment & SPLIT_WORD;
		
		uint8_t max_y = y + height;
		bool last_line;
		int tmp_ptr;
		do {
				last_line = !multiline || (y + 2 * font->height + font->charDist > max_y);
				tmp_ptr = ptr;
				uint8_t text_width = calc_text_width(text, &tmp_ptr, font_type, split_word || !multiline, width);
				uint8_t max_x = x + text_width;
			
				bool first_char = true;
				while ((c = u8_nextchar(text, &ptr)) > 0) {
						if (first_char && is_whitespace(c)) {
								continue;
						}
						first_char = false;
						uint8_t char_width = calc_char_width(c, font);
						if (x + char_width > max_x) {
								//overflow
								ptr = prev_ptr;
								break;
						}
					
						x += char_width;
						x += font->charDist;
						prev_ptr = ptr;
				}
				if (c <= 0){
						last_line = true;
				}
				
				x = start_x;
				y += font->height;
				if(!last_line) {
						y += (c==11 ? font->height/2 : font->charDist);
				}
		} while(!last_line);
				
		return y - start_y;
}

uint_fast8_t mlcd_draw_text(const char *text, uint_fast8_t start_x, uint_fast8_t start_y, uint_fast8_t width, uint_fast8_t height, uint_fast8_t font_type, uint8_t font_alignment) {
	  int ptr = 0;
		int prev_ptr = 0;
	  uint32_t c;
		
		uint8_t x = start_x;
		uint8_t y = start_y;
	  if (width == NULL) {
			  width = MLCD_XRES - x;
		}
	  if (height == NULL) {
			  height = MLCD_YRES - y;
		}
		uint8_t max_y = y + height;
	  const FONT_INFO* font = mlcd_resolve_font(font_type);
	  bool multiline = font_alignment & MULTILINE;
	  bool split_word = font_alignment & SPLIT_WORD;
		bool line_through = font_alignment & STYLE_LINE_THROUGH;
		bool underline = font_alignment & STYLE_UNDERLINE;
		
		if (font_alignment & VERTICAL_ALIGN_CENTER) {
				int calc_height = mlcd_calc_text_height(text, start_x, start_y, width, height, font_type, font_alignment);
				if (calc_height < height) {
						y += (height-calc_height)/2;
				}
		} else if (font_alignment & VERTICAL_ALIGN_BOTTOM) {
				int calc_height = mlcd_calc_text_height(text, start_x, start_y, width, height, font_type, font_alignment);
				if (calc_height < height) {
						y += height-calc_height;
				}
		}
		bool last_line;
		int tmp_ptr;
		do {
				last_line = !multiline || (y + 2 * font->height + font->charDist > max_y);
				tmp_ptr = ptr;
				uint8_t text_width = calc_text_width(text, &tmp_ptr, font_type, split_word || !multiline, width);
				if (font_alignment & HORIZONTAL_ALIGN_CENTER) {
						if (text_width < width) {
								x += (width - text_width)/2;
						}
				} else if (font_alignment & HORIZONTAL_ALIGN_RIGHT) {
						if (text_width < width) {
								x += width - text_width;
						}
				}
				uint8_t max_x = x + text_width;
			
				bool first_char = true;
				while ((c = u8_nextchar(text, &ptr)) > 0) {
						if (first_char && is_whitespace(c)) {
								continue;
						}
						first_char = false;
						uint8_t char_width = calc_char_width(c, font);
						if (x + char_width > max_x) {
								//overflow
								ptr = prev_ptr;
								break;
						}
					
						x += mlcd_draw_char(c, x, y, max_x - x, max_y - y, font);
						x += font->charDist;
						prev_ptr = ptr;
				}
				if (c <= 0){
						last_line = true;
				}
				if (line_through)
					fillRectangle(start_x, y+(font->height >> 1)-2, x-start_x, 2);
				if (underline)
					fillRectangle(start_x, y+font->height-2, x-start_x, 2);
				
				x = start_x;
				y += font->height + (c==11 ? font->height/2 : font->charDist);
		} while(!last_line);
				
		return 0;
}

