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
    if (updateSands()) {
        refreshScreen(drawSands);
    }
    //_delay_ms(DELAY_LOOP);
}
