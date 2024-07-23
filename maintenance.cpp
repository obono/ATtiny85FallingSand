#include "common.h"

/*  Defines  */

#define PHASE_0     0
#define PHASE_1     32
#define PHASE_2     128
#define PHASE_3     160

#define ONE_GRAVITY 64

/*  Local Functions  */

static void drawChar(int8_t x, int8_t y, char c);
static void drawNumber(int8_t x, int8_t y, int16_t n);
static void drawString(int8_t y, const void *p);

/*  Local Functions (macros)  */

/*  Local Constants  */

PROGMEM static const uint16_t font[] = {
                                                     00002,
    02112, 01221, 05250, 02720, 01200, 00600, 02000, 01224,
    02552, 02223, 07243, 03427, 04754, 03437, 03571, 01247,
    07576, 04657, 02002, 01202, 04240, 06060, 02420, 02047,
    07777, 05752, 03537, 06116, 03553, 07137, 01317, 06516,
    05575, 02222, 02544, 05535, 07111, 05577, 05553, 03556,
    01353, 02756, 05353, 03636, 02227, 07555, 02555, 07755,
    05525, 02255, 07367,
};

/*  Local Variables  */

static XYZ_T tempAccel, lastAccel;
static uint16_t *canvas;
static bool isStill;

/*---------------------------------------------------------------------------*/

void initMaintenance(void)
{
    clearXYZ(tempAccel);
    clearXYZ(lastAccel);
    isStill = false;
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
        isStill = (abs(tempAccel.value.x - lastAccel.value.x) <= 1
                && abs(tempAccel.value.y - lastAccel.value.y) <= 1
                && abs(tempAccel.value.z - lastAccel.value.z) <= 1);
        lastAccel = tempAccel;
        clearXYZ(tempAccel);

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
                    if (abs(lastAccel.value.x) <= 1 && abs(lastAccel.value.y) <= 1
                            && abs(lastAccel.value.z - ONE_GRAVITY) <= 1) {
                        saveConfig();
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
}

void drawMaintenance(int16_t y, uint8_t *pBuffer)
{
    memset(pBuffer, 0, PAGE_DATA_LENGTH);
    canvas = (uint16_t *)pBuffer;
    if (counter < PHASE_1) {
        drawChar(0, 1 - y, 'X');
        drawNumber(13, 1 - y, lastAccel.value.x);
        drawChar(0, 6 - y, 'Y');
        drawNumber(13, 6 - y, lastAccel.value.y);
        drawChar(0, 11 - y, 'Z');
        drawNumber(13, 11 - y, lastAccel.value.z);
    } else if (counter < PHASE_2) {
        if (counter & 4) drawString(3 - y, F("KEEPFLAT"));
    } else {
        drawString(1 - y, F("CALIBRATING"));
        if (counter & 2) drawChar(12, 11 - y, '@');
    }
}

/*---------------------------------------------------------------------------*/

static void drawChar(int8_t x, int8_t y, char c)
{
    if (x <= -3 || x >= BOX_SIZE || y <= -4 || y >= PAGE_HEIGHT || c < '\'' || c > 'Z') return;
    uint16_t ptn = pgm_read_word(&font[c - '\'']);
    for (uint8_t i = 0; i < 4 && y < PAGE_HEIGHT; i++, y++) {
        if (y >= 0) canvas[y] |= (ptn >> (i * 3) & 7) << x;
    }
}

static void drawNumber(int8_t x, int8_t y, int16_t n)
{
    int16_t a = abs(n);
    do {
        drawChar(x, y, '0' + a % 10);
        a /= 10;
        x -= 4;
    } while (a > 0);
    if (n < 0) drawChar(x, y, '-');
}

static void drawString(int8_t y, const void *p)
{
    int8_t x = 0;
    char c;
    while ((c = pgm_read_byte(p++)) && y <= PAGE_HEIGHT) {
        drawChar(x, y, c);
        x += 4;
        if (x >= BOX_SIZE) {
            x = 0;
            y += 5;
        }
    }
}
