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

/*  Local Variables  */

static XYZ_T accel;

/*---------------------------------------------------------------------------*/

void initCore(void)
{
    SIMPLEWIRE::begin();
    delay(200);
    uint8_t buf[7];

    // Setup matrix LED
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
    buf[0] = MMA7455_REG_XOFFL;
    readConfig(0, &buf[1], 6);
    SIMPLEWIRE::write(MMA7455_ADDRESS, buf, 7);
    memset(&accel, 0, sizeof(accel));
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

void getAcceleration(int16_t *pX, int16_t *pY, int16_t *pZ)
{
    uint8_t reg, buf[6];
#if 0
    reg = MMA7455_REG_STATUS;
    do {
        SIMPLEWIRE::write(MMA7455_ADDRESS, &reg, 1);
        SIMPLEWIRE::read(MMA7455_ADDRESS, buf, 1);
    } while (!bitRead(buf[0], MMA7455_BIT_DRDY));
#endif

#if 1
    reg = MMA7455_REG_XOUTL;
    SIMPLEWIRE::write(MMA7455_ADDRESS, &reg, 1);
    if (SIMPLEWIRE::read(MMA7455_ADDRESS, (uint8_t *)&accel, 6) == 6) {
        if (accel.reg.x_msb & 0x02) accel.reg.x_msb |= 0xfc;
        if (accel.reg.y_msb & 0x02) accel.reg.y_msb |= 0xfc;
        if (accel.reg.z_msb & 0x02) accel.reg.z_msb |= 0xfc;
    }
#else
    reg = MMA7455_REG_XOUT8;
    SIMPLEWIRE::write(MMA7455_ADDRESS, &reg, 1);
    if (SIMPLEWIRE::read(MMA7455_ADDRESS, buf, 3) == 3) {
        accel.value.x = (int8_t)buf[0];
        accel.value.y = (int8_t)buf[1];
        accel.value.z = (int8_t)buf[2];
    }
#endif
    *pX = accel.value.x;
    *pY = accel.value.y;
    *pZ = accel.value.z;
}

/*---------------------------------------------------------------------------*/

void readConfig(uint16_t addr, void *pConfig, size_t size)
{
    eeprom_busy_wait();
    eeprom_read_block(pConfig, (const void *)addr, size);
}

void writeConfig(uint16_t addr, const void *pConfig, size_t size)
{
    eeprom_busy_wait();
    eeprom_write_block(pConfig, (void *)addr, size);
}
