#pragma once

#include <Arduino.h>

/*  Defines  */

#define BOX_SIZE            16
#define PAGE_HEIGHT         8
#define PAGE_DATA_LENGTH    16

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

/*  Global Functions  */

void    initCore(void);
void    refreshScreen(void (*func)(int16_t, uint8_t *));
void    getAcceleration(int16_t *pX, int16_t *pY, int16_t *pZ);
void    readConfig(uint16_t addr, void *pConfig, size_t size);
void    writeConfig(uint16_t addr, const void *pConfig, size_t size);

void    initSands(void);
void    updateSands(void);
void    drawSands(int16_t y, uint8_t *pBuffer);
