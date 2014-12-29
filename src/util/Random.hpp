#pragma once

#include <stdlib.h>

float randf() { return rand() / (float)RAND_MAX; }
double randd() { return rand() / (double)RAND_MAX; }

bool randprob(float probability) { return randd() <= probability; }
bool randprob(double probability) { return randd() <= probability; }