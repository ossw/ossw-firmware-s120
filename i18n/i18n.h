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
#define MESSAGE_MODE_OFFLINE 10
#define MESSAGE_INCOMING_CALL 11
#define MESSAGE_FORMAT 12
#define MESSAGE_NO_FILES_FOUND 13
#define MESSAGE_REMOVE 14
#define MESSAGE_YES 15
#define MESSAGE_NO 16
#define MESSAGE_ABOUT 17
#define MESSAGE_ALARM_CLOCK 18
#define MESSAGE_ALARM_SNOOZE 19
#define MESSAGE_ALARM_DISMISS 20
#define MESSAGE_SUNDAY_1 21
#define MESSAGE_SUNDAY_3 28
#define MESSAGE_JANUARY_3 35
#define MESSAGE_RTC_REFRESH 47
#define MESSAGE_1_SECOND 48
#define MESSAGE_1_MINUTE 49
#define MESSAGE_SHAKE_LIGHT 50
#define MESSAGE_NOTIF_LIGHT 51
#define MESSAGE_TIMER				52
#define MESSAGE_HOURS				53
#define MESSAGE_MINUTES			54
#define MESSAGE_SECONDS			55

typedef struct
{
	  const char* message;
} I18N_MESSAGE;	

const char* i18n_resolve_message(uint16_t message_key);

#define I18N_TRANSLATE(KEY) i18n_resolve_message(KEY)

#endif
