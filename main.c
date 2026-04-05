#include <stdlib.h>
#include <time.h>
#include "system/scheduler.h"

int main() {
    srand(time(NULL));
    start_system();
    return 0;
}