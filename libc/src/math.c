#include "../include/math.h"

double max(double x, double y) {
    return x > y ? x : y;
}

double min(double x, double y) {
    return x < y ? x : y;
}

int abs(int x) {
    return x < 0 ? -x : x;
}
