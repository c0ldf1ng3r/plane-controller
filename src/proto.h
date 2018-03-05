#include <sys/types.h>


#define CHANNEL_COUNT 8
#define UNUSED_SERVO -1
#define PIN_UP 9999
#define PIN_DOWN 8888

struct PCK{
	int32_t channels[CHANNEL_COUNT];	
} __attribute__((__packed__));
