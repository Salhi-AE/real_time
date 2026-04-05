#include "strategy.h"

int generate_signal(double price) {
    if (price < 64900) return 1;   // BUY
    if (price > 65100) return -1;  // SELL
    return 0;
}