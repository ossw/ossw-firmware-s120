#include "mlcd_text.h"
#include "default_font.h"
#include "mlcd.h"
#include "utf8.h"

static const FONT_INFO *font = &DEFAULT_FONT;

uint_fast8_t mlcd_draw_text(char *text, uint_fast8_t x, uint_fast8_t y, uint_fast8_t size) {
	  int ptr = 0;
	  uint32_t c;
	  while ((c = u8_nextchar(text, &ptr)) > 0) {
			  x += mlcd_draw_char(c, x, y, size);
			  x += font->charDist;
		}
		return 0;
}

uint_fast8_t mlcd_draw_char(uint32_t c, uint_fast8_t x, uint_fast8_t y, uint_fast8_t size){

	 if (c == ' ') {
		   return font->spaceWidth;
	 }
	
   if ((c < font->startChar) || (c > font->endChar)) 
      return 0; 

	 const FONT_CHAR_INFO_LOOKUP *matchingTable = 0;
	 for (int t = 0; t < DEFAULT_FONT_LOOKUP_TABLES_NO; t++) {
	    const FONT_CHAR_INFO_LOOKUP *table = &(font->charInfoLookup[t]);
		  if (c >= table->startChar && c <= table->endChar) {
				  matchingTable = table;
			}
	 }
	 if (!matchingTable) {
		   return 0;
	 }
	 
   uint32_t charIndex = c - matchingTable->startChar;  //Character index 
   const FONT_CHAR_INFO *charInfo = matchingTable->charInfo;          //Point to start of descriptors 
   charInfo += charIndex;              //Point to current char info 

   const uint8_t *bitmapPointer = font->data;                   //Point to start of bitmaps 
   bitmapPointer += charInfo->offset;            //Point to start of c bitmap 

	 mlcd_fb_draw_bitmap(bitmapPointer, x, y, charInfo->width, font->height);
   
   return charInfo->width; 
}
