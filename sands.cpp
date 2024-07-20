#include "common.h"

/*  Defines  */

/*  Local Functions  */

static void moveSands(int16_t vx, int16_t vy);
static void moveSand(int8_t x, int8_t y, int16_t vx, int16_t vy);
static int8_t ternaryRandom(int16_t r);

/*  Local Functions (macros)  */

#define mersenneRandom(n)   (random() & ((1 << n) - 1))

/*  Local Variables  */

static uint16_t sands[BOX_SIZE];

/*---------------------------------------------------------------------------*/

void initSands(void)
{
    uint8_t r = mersenneRandom(5) + 16;
    for (uint8_t y = 0; y < BOX_SIZE; y++) {
        sands[y] = 0;
        for (uint8_t x = 0; x < BOX_SIZE; x++) {
            if (mersenneRandom(6) < r) bitSet(sands[y], x);
        }
    }
}

void updateSands(void)
{
    int16_t vx, vy, vz;
    getAcceleration(&vx, &vy, &vz);
    moveSands(vx, -vy);
    if (digitalRead(1) == LOW) bitSet(sands[0], 0);
    if (digitalRead(3) == LOW) bitClear(sands[0], 0);
}

void drawSands(int16_t y, uint8_t *pBuffer)
{
    memcpy(pBuffer, (uint8_t*)(&sands[y]), PAGE_DATA_LENGTH);
}

/*---------------------------------------------------------------------------*/

static void moveSands(int16_t vx, int16_t vy)
{
    for (uint8_t i = 0; i < BOX_SIZE; i++) {
        int8_t y = (vy < 0) ? i : BOX_SIZE - 1 - i;
        for (uint8_t j = 0; j < BOX_SIZE; j++) {
            int8_t x = (vx < 0) ? j : BOX_SIZE - 1 - j;
            if (bitRead(sands[y], x)) moveSand(x, y, vx, vy);
        }
    }
}

static void moveSand(int8_t x, int8_t y, int16_t vx, int16_t vy)
{
    int8_t dx = x + ternaryRandom(vx);
    int8_t dy = y + ternaryRandom(vy);
    if (dx < 0 || dx >= BOX_SIZE) dx = x;
    if (dy < 0 || dy >= BOX_SIZE) dy = y;
    if (bitRead(sands[dy], dx)) {
        if (!(dx != x && dy != y)) return;
        if (bitRead(sands[y], dx)) dx = x;
        if (bitRead(sands[dy], x)) dy = y;
        if (dx == x && dy == y) return;
        if (dx != x && dy != y) {
            if (mersenneRandom(1)) {
                dx = x;
            } else {
                dy = y;
            }
        }
    }
    bitClear(sands[y], x);
    bitSet(sands[dy], dx);
}

static int8_t ternaryRandom(int16_t r)
{
    r += mersenneRandom(4) - 8;
    if (mersenneRandom(6) >= abs(r)) return 0;
    return (r > 0) ? 1 : -1;
}
