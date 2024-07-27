#pragma once

#include <Arduino.h>

/*  Defines  */

#define BOX_SIZE            16
#define PAGE_HEIGHT         8
#define PAGE_DATA_LENGTH    16

#define WALL_TYPE_MAX       9
#define AMOUNT_TYPE_MAX     6
#define UP_BUTTON           bit(PINB3)
#define DOWN_BUTTON         bit(PINB1)

#define EEPROM_CONFIG_ADDR  0

/*  Typedefs  */

union XYZ_T {
    struct {
        uint8_t x_lsb;
        uint8_t x_msb;
        uint8_t y_lsb;
        uint8_t y_msb;
        uint8_t z_lsb;
        uint8_t z_msb;
    } reg;
    struct {
        int16_t x;
        int16_t y;
        int16_t z;
    } value;
};

typedef struct {
    XYZ_T   offset;
    uint8_t wallType;
    uint8_t amountType;
} CONFIG_T;

/*  Global Functions  */

void    initCore(void);
void    updateButtonState(void);
uint8_t isButtonPressed(uint8_t button);
uint8_t isButtonDown(uint8_t button);
void    refreshScreen(void (*func)(int16_t, uint8_t *));
XYZ_T   *getAcceleration(void);
void    setAccelerationOffset(XYZ_T *pOffset);
void    saveConfig(void);

void    initSands(void);
void    updateSands(void);
void    drawSands(int16_t y, uint8_t *pBuffer);
void    initMaintenance(void);
void    updateMaintenance(void);
void    drawMaintenance(int16_t y, uint8_t *pBuffer);

/*  Global Functions (macros)  */

#define clearXYZ(xyz)   memset(&xyz, 0, sizeof(XYZ_T))

/*  Global Variables  */

extern CONFIG_T config;
extern uint8_t  counter;
