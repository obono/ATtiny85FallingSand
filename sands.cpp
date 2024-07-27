#include "common.h"

/*  Defines  */

#define COUNTER_FREEZE  30

/*  Local Functions  */

static void arrangeWalls(uint8_t wallType);
static void scatterSands(uint8_t amountType, uint8_t wallType);
static void moveSands(int16_t vx, int16_t vy);
static void moveSand(int8_t x, int8_t y, int16_t vx, int16_t vy);
static int8_t ternaryRandom(int16_t r);

/*  Local Functions (macros)  */

#define mersenneRandom(n)   (random() & ((1 << n) - 1))

/*  Local Constants  */

PROGMEM static const uint16_t wallPattern[WALL_TYPE_MAX][BOX_SIZE] = {
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Empty
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    },{
        0x0000, 0x0000, 0x0810, 0x0810, 0x3e7c, 0x0810, 0x0810, 0x0000, // ++++
        0x0000, 0x0810, 0x0810, 0x3e7c, 0x0810, 0x0810, 0x0000, 0x0000
    },{
        0x0000, 0x0000, 0x0000, 0x3ffc, 0x0000, 0x0000, 0x0000, 0xfe7f, // Layers
        0x0000, 0x0000, 0x0000, 0x3ffc, 0x0000, 0x0000, 0x0000, 0x0000
    },{
        0x0000, 0x0000, 0x0240, 0x0240, 0x0660, 0x0420, 0x0c30, 0x0810, // Flask
        0x1818, 0x1008, 0x300c, 0x2004, 0x6006, 0x4002, 0x7ffe, 0x0000
    },{
        0x0000, 0x0000, 0xfffc, 0x0004, 0x0004, 0x3fe4, 0x2024, 0x2024, // Swirl
        0x2424, 0x2424, 0x27e4, 0x2004, 0x2004, 0x3ffc, 0x0000, 0x0000
    },{
        0x0000, 0x0240, 0x0240, 0x03c0, 0x4812, 0x4812, 0x781e, 0x0240, // 7 cups
        0x0240, 0x03c0, 0x4812, 0x4812, 0x781e, 0x0240, 0x0240, 0x03c0
    },{
        0xc001, 0x6003, 0x3006, 0x180c, 0x0c18, 0x0630, 0x0020, 0x0000, // X
        0x0000, 0x0400, 0x0c60, 0x1830, 0x3018, 0x600c, 0xc006, 0x8003
    },{
        0x0802, 0xc810, 0x0b90, 0x4017, 0x4080, 0x5c81, 0x00b9, 0x0401, // Mesh
        0xe408, 0x05c8, 0x200b, 0x2040, 0x2e40, 0x005c, 0x0200, 0x7204
    },{
        0x0004, 0x0004, 0x3ffc, 0x2000, 0x2000, 0x2000, 0x3ffc, 0x0004, // Separated
        0x0004, 0x3ffc, 0x2000, 0x2000, 0x2000, 0x3ffc, 0x0004, 0x0004
    }
};

/*  Local Variables  */

static uint16_t sands[BOX_SIZE];
static bool     isScattered;

/*---------------------------------------------------------------------------*/

void initSands(void)
{
    arrangeWalls(config.wallType);
    scatterSands(config.amountType, config.wallType);
    counter = 0;
}

void updateSands(void)
{
    if (isButtonDown(UP_BUTTON)) {
        if (++config.wallType == WALL_TYPE_MAX) config.wallType = 0;
        arrangeWalls(config.wallType);
        if (config.wallType == 0) scatterSands(config.amountType, config.wallType);
        counter = COUNTER_FREEZE;
    } else if (isButtonDown(DOWN_BUTTON)) {
        if (++config.amountType == AMOUNT_TYPE_MAX) config.amountType = 0;
        arrangeWalls(config.wallType);
        scatterSands(config.amountType, config.wallType);
        counter = COUNTER_FREEZE;
    }

    if (counter > 0) {
        if (--counter == 0) {
            if (!isScattered) scatterSands(config.amountType, config.wallType);
            saveConfig();
        }
    } else {
        XYZ_T *pAccel = getAcceleration();
        moveSands(pAccel->value.x, -pAccel->value.y);
    }
}

void drawSands(int16_t y, uint8_t *pBuffer)
{
    memcpy(pBuffer, (uint8_t*)(&sands[y]), PAGE_DATA_LENGTH);
}

/*---------------------------------------------------------------------------*/

static void arrangeWalls(uint8_t wallType)
{
    memcpy_P(sands, wallPattern[wallType], sizeof(sands));
    isScattered = false;
}

static void scatterSands(uint8_t amountType, uint8_t wallType)
{
    uint8_t amount = (amountType < 3) ? 1 + amountType : 5 - amountType;
    amount = (wallType == 0) ? 48 + (amount << 5) : 32 + (amount << 4);

    for (uint8_t y = 0; y < BOX_SIZE; y++) {
        for (uint8_t x = 0; x < BOX_SIZE; x++) {
            if (mersenneRandom(8) < amount) bitSet(sands[y], x);
        }
    }
    isScattered = true;
}

static void moveSands(int16_t vx, int16_t vy)
{
    for (uint8_t i = 0; i < BOX_SIZE; i++) {
        int8_t y = (vy < 0) ? i : BOX_SIZE - 1 - i;
        uint16_t wall = pgm_read_word(&wallPattern[config.wallType][y]);
        for (uint8_t j = 0; j < BOX_SIZE; j++) {
            int8_t x = (vx < 0) ? j : BOX_SIZE - 1 - j;
            if (bitRead(sands[y], x) && !bitRead(wall, x)) moveSand(x, y, vx, vy);
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
