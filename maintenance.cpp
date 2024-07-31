#include "common.h"

/*  Defines  */

#define PHASE_0         0
#define PHASE_1         32
#define PHASE_2         128
#define PHASE_3         160

#define ONE_GRAVITY     64
#define DIFF_THRESHOLD  2

/*  Local Functions  */

static void controlMaintenance(bool isStill);
static void drawMaintenance(void);

/*  Local Variables  */

static XYZ_T tempAccel, lastAccel;

/*---------------------------------------------------------------------------*/

void initMaintenance(void)
{
    clearXYZ(tempAccel);
    clearXYZ(lastAccel);
    counter = PHASE_0;
}

void updateMaintenance(void)
{
    XYZ_T *pAccel = getAcceleration();
    tempAccel.value.x += pAccel->value.x;
    tempAccel.value.y += pAccel->value.y;
    tempAccel.value.z += pAccel->value.z;

    if ((++counter & 7) == 0) {
        tempAccel.value.x >>= 3;
        tempAccel.value.y >>= 3;
        tempAccel.value.z >>= 3;
        bool isStill = (abs(tempAccel.value.x - lastAccel.value.x) +
                abs(tempAccel.value.y - lastAccel.value.y) +
                abs(tempAccel.value.z - lastAccel.value.z) <= DIFF_THRESHOLD);
        lastAccel = tempAccel;
        clearXYZ(tempAccel);
        controlMaintenance(isStill);
    }
    drawMaintenance();
}

/*---------------------------------------------------------------------------*/

static void controlMaintenance(bool isStill)
{
    if (counter <= PHASE_1) {
        if (!isButtonPressed(UP_BUTTON | DOWN_BUTTON)) counter = PHASE_0;
    } else if (counter <= PHASE_2) {
        if (isStill) {
            if (counter == PHASE_2) {
                clearXYZ(config.offset);
                setAccelerationOffset(&config.offset);
            }
        } else {
            counter = PHASE_1;
        }
    } else {
        if (isStill) {
            if (counter == PHASE_3) {
                if (abs(lastAccel.value.x) + abs(lastAccel.value.y) +
                        abs(lastAccel.value.z - ONE_GRAVITY) <= DIFF_THRESHOLD) {
                    saveConfig(EEPROM_ADDR_OFFSET, &config.offset, sizeof(XYZ_T));
                    counter = PHASE_0;
                } else {
                    config.offset.value.x -= lastAccel.value.x * 2;
                    config.offset.value.y -= lastAccel.value.y * 2;
                    config.offset.value.z -= (lastAccel.value.z - ONE_GRAVITY) * 2;
                    setAccelerationOffset(&config.offset);
                    counter = PHASE_2;
                }
            }
        } else {
            counter = PHASE_2;
        }
    }
}

static void drawMaintenance(void)
{
    clearScreen();
    if (counter < PHASE_1) {
        for (uint8_t i = 0; i < 3; i++) {
            int8_t y = i * 5 + 1;
            drawChar(0, y, 'X' + i);
            drawNumber(13, y, (&lastAccel.value.x)[i]);
        }
    } else if (counter < PHASE_2) {
        if (counter & 4) drawString(3, F("KEEPFLAT"));
    } else {
        drawString(1, F("CALIBRATING"));
        if (counter & 2) drawChar(12, 11, '@');
    }
}
