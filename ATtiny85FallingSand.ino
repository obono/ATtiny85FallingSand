#include <util/delay.h>
#include "common.h"

/*  Defines  */

#define DELAY_LOOP  30

/*---------------------------------------------------------------------------*/

void setup(void)
{
    initCore();
    initSands();
    refreshScreen(drawSands);
}

void loop(void)
{
    long now = millis();
    updateSands();
    refreshScreen(drawSands);
    long wait = now + DELAY_LOOP - millis();
    if (wait > 0) delay(wait);
}
