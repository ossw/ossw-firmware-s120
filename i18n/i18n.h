#ifndef I18N_H
#define I18N_H

#include <stdint.h>

#define LANGUAGE_ENGLISH 0

#define MESSAGE_UNKNOWN 0
#define MESSAGE_DATE 1
#define MESSAGE_TIME 2
#define MESSAGE_DISPLAY 3
#define MESSAGE_SENSORS 4
#define MESSAGE_RESTART 5
#define MESSAGE_SET_DATE 6
#define MESSAGE_SET_TIME 7
#define MESSAGE_MODE_PERIPHERAL 8
#define MESSAGE_MODE_CENTRAL 9
#define MESSAGE_MODE_AIRPLANE 10

typedef struct
{
	  const char* message;
} I18N_MESSAGE;	

const char* i18n_resolve_message(uint16_t message_key);

#define I18N_TRANSLATE(KEY) i18n_resolve_message(KEY)

#endif
