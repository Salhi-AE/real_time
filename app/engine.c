#include "../core/strategy.h"
#include "../infra/data_writer.h"
#include <stdio.h>

static double balance = 1000;

void process_price(double price) {
    int signal = generate_signal(price);

    if (signal == 1) {
        balance -= 100;
        printf("BUY @ %.2f\n", price);
    }
    else if (signal == -1) {
        balance += 100;
        printf("SELL @ %.2f\n", price);
    }

    write_data(price, balance);
}