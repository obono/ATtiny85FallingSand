#include "common.h"

/*  Defines  */

enum : uint8_t {
    SAND_EMPTY = 0,
    SAND_NORMAL,
    SAND_FIXED,
};

#define RANDOM_QUIVER       8
#define RANDOM_THRESHOLD    64

/*  Local Functions  */

static void moveSands(int16_t vx, int16_t vy);
static void moveSand(int8_t x, int8_t y, int16_t vx, int16_t vy);
static uint8_t getSand(int8_t x, int8_t y);
static void setSand(int8_t x, int8_t y, uint8_t s);
static int8_t ternaryRandom(int16_t r);

/*  Local Variables  */

static uint16_t sands[BOX_SIZE];

/*---------------------------------------------------------------------------*/

void initSands(void)
{
    uint8_t r = random(16, 48);
    for (uint8_t y = 0; y < BOX_SIZE; y++) {
        for (uint8_t x = 0; x < BOX_SIZE; x++) {
            setSand(x, y, (random(64) < r) ? SAND_NORMAL : SAND_EMPTY);
        }
    }
}

bool updateSands(void)
{
    int16_t vx, vy, vz;
    getAcceleration(&vx, &vy, &vz);
    moveSands(vx, -vy);
    if (digitalRead(1) == LOW) setSand(0, 0, SAND_NORMAL);
    if (digitalRead(3) == LOW) setSand(0, 0, SAND_EMPTY);
    return true;
}

void drawSands(int16_t y, uint8_t *pBuffer)
{
    memcpy(pBuffer, (uint8_t*)(&sands[y]), sizeof(sands[0]) * PAGE_HEIGHT);
}

/*---------------------------------------------------------------------------*/

static void moveSands(int16_t vx, int16_t vy)
{
    for (uint8_t i = 0; i < BOX_SIZE; i++) {
        int8_t y = (vy < 0) ? i : BOX_SIZE - 1 - i;
        for (uint8_t j = 0; j < BOX_SIZE; j++) {
            int8_t x = (vx < 0) ? j : BOX_SIZE - 1 - j;
            if (getSand(x, y) == SAND_NORMAL) moveSand(x, y, vx, vy);
        }
    }
}

static void moveSand(int8_t x, int8_t y, int16_t vx, int16_t vy)
{
    int8_t dx = ternaryRandom(vx);
    int8_t dy = ternaryRandom(vy);
    if (dx == 0 && dy == 0) return;
    if (dx != 0 && dy != 0) {
        if (getSand(x + dx, y + dy) != SAND_EMPTY) {
            if (getSand(x + dx, y) != SAND_EMPTY) dx = 0;
            if (getSand(x, y + dy) != SAND_EMPTY) dy = 0;
            if (dx == 0 && dy == 0) return;
            if (dx != 0 && dy != 0) {
                if (random(2)) {
                    dx = 0;
                } else {
                    dy = 0;
                }
            }
        }
    } else {
        if (getSand(x + dx, y + dy) != SAND_EMPTY) return;
    }
    setSand(x, y, SAND_EMPTY);
    setSand(x + dx, y + dy, SAND_NORMAL);
}

static uint8_t getSand(int8_t x, int8_t y)
{
    if (x < 0 || y < 0 || x >= BOX_SIZE || y >= BOX_SIZE) return SAND_FIXED;
    return (bitRead(sands[y], x)) ? SAND_NORMAL : SAND_EMPTY;
}

static void setSand(int8_t x, int8_t y, uint8_t s)
{
    if (x < 0 || y < 0 || x >= BOX_SIZE || y >= BOX_SIZE) return;
    if (s == SAND_EMPTY) {
        bitClear(sands[y], x);
    } else if (s == SAND_NORMAL) {
        bitSet(sands[y], x);
    }
}

static int8_t ternaryRandom(int16_t r)
{
    r += random(-RANDOM_QUIVER, RANDOM_QUIVER + 1);
    if (random(RANDOM_THRESHOLD) >= abs(r)) return 0;
    return (r > 0) ? 1 : -1;
}
