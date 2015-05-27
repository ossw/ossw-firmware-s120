#include "i18n.h"
#include "messages_en.h"

static const uint8_t current_language = LANGUAGE_ENGLISH;

static const I18N_MESSAGE* messages[] = {
	  /* 0 */ messages_en
};

const char* i18n_resolve_message(uint16_t message_key) {
	  return messages[current_language][message_key].message;
}
