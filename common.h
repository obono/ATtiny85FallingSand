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

#define EEPROM_ADDR_CONFIG  0
#define EEPROM_ADDR_OFFSET  EEPROM_ADDR_CONFIG
#define EEPROM_ADDR_TYPES   (EEPROM_ADDR_OFFSET + sizeof(XYZ_T))
#define EEPROM_SIZE_TYPES   2

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

void    clearScreen(void);
void    drawChar(int8_t x, int8_t y, char c);
void    drawNumber(int8_t x, int8_t y, int16_t n);
void    drawString(int8_t y, const void *p);
void    refreshScreen(void);

XYZ_T   *getAcceleration(void);
void    setAccelerationOffset(XYZ_T *pOffset);
void    saveConfig(uint8_t addr, void *data, size_t len);

void    initSands(void);
void    updateSands(void);
void    initMaintenance(void);
void    updateMaintenance(void);

/*  Global Functions (macros)  */

#define clearXYZ(xyz)   memset(&xyz, 0, sizeof(XYZ_T))

/*  Global Variables  */

extern CONFIG_T config;
extern uint16_t screenBuffer[BOX_SIZE];
extern uint8_t  counter;
