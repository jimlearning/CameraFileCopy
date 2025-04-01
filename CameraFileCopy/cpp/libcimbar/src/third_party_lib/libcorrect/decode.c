#include <stdlib.h>
#include <string.h>
#include "sse_compat.h"

ssize_t decode(void *context, uint8_t *input, size_t input_len, uint8_t *output) {
    // TODO: Implement iOS-compatible decode logic
    return 0;
}

// Define missing polynomial constants used in tests
#define V27POLYA 0x1A
#define V27POLYB 0x1B
