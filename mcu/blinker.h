#if !defined(BLINKER_H)
#define BLINKER_H

#include "al.h"

void blinker_set_l(bool enabled);

void blinker_set_r(bool enabled);

void blinker_task(void *params);

#endif // BLINKER_H
