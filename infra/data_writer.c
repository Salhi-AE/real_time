#include <stdio.h>
#include "data_writer.h"

void write_data(double price, double balance) {
    FILE *f = fopen("backend/data.json", "w");
    if (!f) return;

    fprintf(f, "{ \"price\": %.2f, \"balance\": %.2f }\n", price, balance);

    fclose(f);
}