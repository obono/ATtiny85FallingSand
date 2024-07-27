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

/*  Global Variables  */

CONFIG_T    config;
uint8_t     counter;

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
    eeprom_read_block(&config, (const void *)EEPROM_CONFIG_ADDR, sizeof(config));
    if (config.wallType >= WALL_TYPE_MAX || config.amountType >= AMOUNT_TYPE_MAX) {
        memset(&config, 0, sizeof(config));
    }

    // Setup matrix LED
    uint8_t buf[3];
    buf[0] = HT16K33_CMD_SETUP;
    buf[1] = HT16K33_CMD_DIMMING;
    buf[2] = HT16K33_CMD_DISPLAY;
    for (uint8_t i = 0; i < BOX_SIZE / PAGE_HEIGHT; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            SIMPLEWIRE::write(HT16K33_ADDRESS + i, &buf[j], 1);
        }
    }

    // Setup acceleration sensor
    buf[0] = MMA7455_REG_MCTL;
    buf[1] = bit(MMA7455_BIT_GLVL0) | bit(MMA7455_BIT_MODE0) | bit(MMA7455_BIT_DRPD);
    SIMPLEWIRE::write(MMA7455_ADDRESS, buf, 2);
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

void refreshScreen(void (*func)(int16_t, uint8_t *))
{
    uint8_t buf[HT16K33_DATA_LENGTH + 1];
    buf[0] = HT16K33_DATA_START;
    for (int16_t y = 0; y < BOX_SIZE; y += PAGE_HEIGHT) {
        if (func) {
            func(y, &buf[1]);
        } else {
            memset(&buf[1], 0, HT16K33_DATA_LENGTH);
        }
        SIMPLEWIRE::write(HT16K33_ADDRESS + (y >> 3), buf, HT16K33_DATA_LENGTH + 1);
    }
}

XYZ_T *getAcceleration(void)
{
    uint8_t reg, ret;
#if 0
    reg = MMA7455_REG_STATUS;
    do {
        SIMPLEWIRE::write(MMA7455_ADDRESS, &reg, 1);
        SIMPLEWIRE::read(MMA7455_ADDRESS, &ret, 1);
    } while (!bitRead(ret, MMA7455_BIT_DRDY));
#endif

    reg = MMA7455_REG_XOUTL;
    SIMPLEWIRE::write(MMA7455_ADDRESS, &reg, 1);
    if (SIMPLEWIRE::read(MMA7455_ADDRESS, (uint8_t *)&accel, sizeof(accel)) > 0) {
        if (accel.reg.x_msb & 0x02) accel.reg.x_msb |= 0xfc;
        if (accel.reg.y_msb & 0x02) accel.reg.y_msb |= 0xfc;
        if (accel.reg.z_msb & 0x02) accel.reg.z_msb |= 0xfc;
    }
    return &accel;
}

void setAccelerationOffset(XYZ_T *pOffset)
{
    uint8_t buf[7];
    buf[0] = MMA7455_REG_XOFFL;
    memcpy(&buf[1], pOffset, sizeof(XYZ_T));
    SIMPLEWIRE::write(MMA7455_ADDRESS, buf, 7);
}

void saveConfig(void)
{
    eeprom_busy_wait();
    eeprom_write_block(&config, (void *)EEPROM_CONFIG_ADDR, sizeof(config));
}
