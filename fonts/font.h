#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// ==========================================================================
// structure definition
// ==========================================================================

// This structure describes a single character's display information
typedef struct
{
	const uint8_t width;				// width, in bits (or pixels), of the character
	const uint16_t offset;					// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array
} FONT_CHAR_INFO;	

typedef struct
{
	const uint16_t 			  startChar;	// the first character in the table
	const uint16_t 			  endChar;		// the last character in the table
	const FONT_CHAR_INFO	*charInfo;		// pointer to array of char information
} FONT_CHAR_INFO_LOOKUP;

// Describes a single font
typedef struct
{
	const uint8_t 			height;	// height of the font's characters
	const uint16_t 			startChar;		// the first character in the font (e.g. in charInfo and data)
	const uint16_t 			endChar;		// the last character in the font
	const uint8_t			  charDist;	// distance between characters
	const uint8_t			  spaceWidth;	// number of pixels that a space character takes up
	const FONT_CHAR_INFO_LOOKUP* charInfoLookup; // character info lookup table
	const uint8_t       lookupTableSize;
	const uint8_t*			data;			// pointer to generated array of character visual representation
} FONT_INFO;

#endif /* FONT_H */
