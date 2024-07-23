#include <util/delay.h>
#include "common.h"

/*  Defines  */

#define DELAY_LOOP  30

/*  Local Variables  */

bool isMaintenance;

/*---------------------------------------------------------------------------*/

void setup(void)
{
    initCore();
    isMaintenance = isButtonPressed(UP_BUTTON | DOWN_BUTTON);
    (isMaintenance) ? initMaintenance() : initSands();
    refreshScreen((isMaintenance) ? drawMaintenance : drawSands);
}

void loop(void)
{
    long now = millis();
    updateButtonState();
    (isMaintenance) ? updateMaintenance() : updateSands();
    refreshScreen((isMaintenance) ? drawMaintenance : drawSands);
    long wait = now + DELAY_LOOP - millis();
    if (wait > 0) delay(wait);
}
