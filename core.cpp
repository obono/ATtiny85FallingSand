#include "common.h"

/*  Defines  */

#ifdef __AVR_ATtiny85__
    #if F_CPU != 16000000UL || not defined DISABLEMILLIS
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

#define LED_ADDRESS         0x70

#define MMA7455_ADDRESS     0x1D
#define MMA7455_REG_XOUTL   0x00
#define MMA7455_REG_XOUT8   0x06
#define MMA7455_REG_STATUS  0x09
#define MMA7455_REG_MCTL    0x16
#define MMA7455_BIT_DRDY    0
#define MMA7455_BIT_MODE0   0
#define MMA7455_BIT_GLVL0   2
#define MMA7455_BIT_DRPD    6

/*  Macro functions  */

/*  Local Variables  */

static int16_t  vx, vy, vz;
static uint8_t  wireBuffer[BOX_SIZE + 1];

/*---------------------------------------------------------------------------*/

void initCore(void)
{
    SIMPLEWIRE::begin();
    delay(200);

    // Setup display
    wireBuffer[0] = 0x21; // system setup
    wireBuffer[1] = 0xe0; // brightness: 0xe0 ~ 0xef
    wireBuffer[2] = 0x81; // display on & blinking off
    for (uint8_t i = 0; i < BOX_SIZE / PAGE_HEIGHT; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            SIMPLEWIRE::write(LED_ADDRESS + i, &wireBuffer[j], 1);
        }
    }

    // Setup acceleration sensor
    wireBuffer[0] = MMA7455_REG_MCTL;
    wireBuffer[1] = bit(MMA7455_BIT_GLVL0) | bit(MMA7455_BIT_MODE0) | bit(MMA7455_BIT_DRPD);
    SIMPLEWIRE::write(MMA7455_ADDRESS, wireBuffer, 2);
    vx = 0;
    vy = 0;
    vz = 0;
}

void refreshScreen(void (*func)(int16_t, uint8_t *))
{
    wireBuffer[0] = 0;
    for (int16_t y = 0; y < BOX_SIZE; y += PAGE_HEIGHT) {
        (func) ? func(y, &wireBuffer[1]) : clearScreenBuffer();
        SIMPLEWIRE::write(LED_ADDRESS + (y >> 3), wireBuffer, BOX_SIZE + 1);
    }
}

void clearScreenBuffer(void)
{
    memset(&wireBuffer[1], 0, BOX_SIZE);
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

#if 0
    reg = MMA7455_REG_XOUTL;
    SIMPLEWIRE::write(MMA7455_ADDRESS, &reg, 1);
    if (SIMPLEWIRE::read(MMA7455_ADDRESS, buf, 6) == 6) {
        vx = buf[0] | buf[1] << 8;
        if (vx & 0x0200) vx |= 0xFC00;
        vy = buf[2] | buf[3] << 8;
        if (vy & 0x0200) vy |= 0xFC00;
        vz = buf[4] | buf[5] << 8;
        if (vz & 0x0200) vz |= 0xFC00;
    }
#else
    reg = MMA7455_REG_XOUT8;
    SIMPLEWIRE::write(MMA7455_ADDRESS, &reg, 1);
    if (SIMPLEWIRE::read(MMA7455_ADDRESS, buf, 3) == 3) {
        vx = (int8_t)buf[0];
        vy = (int8_t)buf[1];
        vz = (int8_t)buf[2];
    }
#endif
    *pX = vx;
    *pY = vy;
    *pZ = vz;
}
