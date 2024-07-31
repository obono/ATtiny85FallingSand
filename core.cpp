#include "common.h"

#include <avr/eeprom.h>

/*  Defines  */

#ifdef __AVR_ATtiny85__
    #if F_CPU != 16000000UL || defined DISABLEMILLIS
        #error Board Configuration is wrong...
    #endif
#else
    #error Sorry, Unsupported Board...
#endif

#define SimpleWire_SCL_PORT B
#define SimpleWire_SCL_POS  2
#define SimpleWire_SDA_PORT B
#define SimpleWire_SDA_POS  0
#include "SimpleWire.h"
#define SIMPLEWIRE          SimpleWire<SimpleWire_1M>

#define HT16K33_ADDRESS     0x70
#define HT16K33_CMD_SETUP   0x21 // normal operation mode
#define HT16K33_CMD_DIMMING 0xe0 // brightness: 0xe0 ~ 0xef
#define HT16K33_CMD_DISPLAY 0x81 // display on & blinking off
#define HT16K33_DATA_START  0x00
#define HT16K33_DATA_LENGTH 0x10

#define MMA7455_ADDRESS     0x1D
#define MMA7455_REG_XOUTL   0x00
#define MMA7455_REG_XOUT8   0x06
#define MMA7455_REG_STATUS  0x09
#define MMA7455_REG_XOFFL   0x10
#define MMA7455_REG_MCTL    0x16
#define MMA7455_BIT_DRDY    0
#define MMA7455_BIT_MODE0   0
#define MMA7455_BIT_GLVL0   2
#define MMA7455_BIT_DRPD    6

#define FONT_W              3
#define FONT_H              4

/*  Global Variables  */

CONFIG_T    config;
uint16_t    screenBuffer[BOX_SIZE];
uint8_t     counter;

/*  Local Constants  */

PROGMEM static const uint16_t font[] = {
                                       00600, 02000, 01224, //      -./
    02552, 02223, 07243, 03427, 04754, 03437, 03571, 01247, // 01234567
    07576, 04657, 02002, 01202, 04240, 06060, 02420, 02047, // 89:;<=>?
    07777, 05752, 03537, 06116, 03553, 07137, 01317, 06516, // @ABCDEFG
    05575, 02222, 02544, 05535, 07111, 05577, 05553, 03556, // HIJKLMNO
    01353, 02756, 05353, 03636, 02227, 07555, 02555, 07755, // PQRSTUVW
    05525, 02255, 07367,                                    // XYZ
};

/*  Local Variables  */

static XYZ_T    accel;
static uint8_t  currentButton, lastButton;

/*---------------------------------------------------------------------------*/

void initCore(void)
{
    SIMPLEWIRE::begin();
    delay(200);
    updateButtonState();

    // Load configuration
    eeprom_busy_wait();
    eeprom_read_block(&config, (const void *)EEPROM_ADDR_CONFIG, sizeof(config));
    if (config.wallType >= WALL_TYPE_MAX || config.amountType >= AMOUNT_TYPE_MAX) {
        memset(&config, 0, sizeof(config));
    }

    // Setup matrix LED
    for (uint8_t i = 0; i < BOX_SIZE / PAGE_HEIGHT; i++) {
        uint8_t addr = HT16K33_ADDRESS + i;
        SIMPLEWIRE::writeWithCommand(addr, HT16K33_CMD_SETUP);
        SIMPLEWIRE::writeWithCommand(addr, HT16K33_CMD_DIMMING);
        SIMPLEWIRE::writeWithCommand(addr, HT16K33_CMD_DISPLAY);
    }

    // Setup acceleration sensor
    uint8_t data = bit(MMA7455_BIT_GLVL0) | bit(MMA7455_BIT_MODE0) | bit(MMA7455_BIT_DRPD);
    SIMPLEWIRE::writeWithCommand(MMA7455_ADDRESS, MMA7455_REG_MCTL, &data, 1);
    setAccelerationOffset(&config.offset);
    clearXYZ(accel);
}

void updateButtonState(void)
{
    lastButton = currentButton;
    currentButton = ~PINB;
}

uint8_t isButtonPressed(uint8_t button)
{
    return currentButton & button;
}

uint8_t isButtonDown(uint8_t button)
{
    return currentButton & ~lastButton & button;
}

/*---------------------------------------------------------------------------*/

void clearScreen(void)
{
    memset(screenBuffer, 0, sizeof(screenBuffer));
}

void drawChar(int8_t x, int8_t y, char c)
{
    if (x <= -3 || x >= BOX_SIZE || y <= -4 || y >= BOX_SIZE || c < '-' || c > 'Z') return;
    uint16_t ptn = pgm_read_word(&font[c - '-']);
    for (uint8_t i = 0; i < 4 && y < BOX_SIZE; i++, y++) {
        if (y >= 0) screenBuffer[y] |= (ptn >> (i * FONT_W) & ((1 << FONT_W) - 1)) << x;
    }
}

void drawNumber(int8_t x, int8_t y, int16_t n)
{
    int16_t a = abs(n);
    do {
        drawChar(x, y, '0' + a % 10);
        a /= 10;
        x -= FONT_W + 1;
    } while (a > 0);
    if (n < 0) drawChar(x, y, '-');
}

void drawString(int8_t y, const void *p)
{
    int8_t x = 0;
    char c;
    while ((c = pgm_read_byte(p++)) && y <= BOX_SIZE) {
        drawChar(x, y, c);
        x += FONT_W + 1;
        if (x >= BOX_SIZE) {
            x = 0;
            y += FONT_H + 1;
        }
    }
}

void refreshScreen(void)
{
    for (uint8_t i = 0; i < BOX_SIZE / PAGE_HEIGHT; i++) {
        uint8_t addr = HT16K33_ADDRESS + i;
        SIMPLEWIRE::writeWithCommand(
                addr, HT16K33_DATA_START, (const uint8_t *)&screenBuffer[i * PAGE_HEIGHT], HT16K33_DATA_LENGTH);
    }
}

/*---------------------------------------------------------------------------*/

XYZ_T *getAcceleration(void)
{
#if 0
    uint8_t ret;
    do {
        SIMPLEWIRE::readWithCommand(MMA7455_ADDRESS, MMA7455_REG_STATUS, &ret, 1);
    } while (!bitRead(ret, MMA7455_BIT_DRDY));
#endif

    if (SIMPLEWIRE::readWithCommand(MMA7455_ADDRESS, MMA7455_REG_XOUTL, (uint8_t *)&accel, sizeof(XYZ_T)) > 0) {
        if (accel.reg.x_msb & 0x02) accel.reg.x_msb |= 0xfc;
        if (accel.reg.y_msb & 0x02) accel.reg.y_msb |= 0xfc;
        if (accel.reg.z_msb & 0x02) accel.reg.z_msb |= 0xfc;
    }
    return &accel;
}

void setAccelerationOffset(XYZ_T *pOffset)
{
    SIMPLEWIRE::writeWithCommand(MMA7455_ADDRESS, MMA7455_REG_XOFFL, (const uint8_t *)pOffset, sizeof(XYZ_T));
}

void saveConfig(uint8_t addr, void *data, size_t len)
{
    eeprom_busy_wait();
    eeprom_write_block(data, (void *)addr, len);
}
