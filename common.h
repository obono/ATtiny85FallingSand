#pragma once

#include <Arduino.h>

/*  Defines  */

#define BOX_SIZE    16
#define PAGE_HEIGHT 8

/*  Global Functions  */

void    initCore(void);
void    refreshScreen(void (*func)(int16_t, uint8_t *));
void    clearScreenBuffer(void);
void    getAcceleration(int16_t *pX, int16_t *pY, int16_t *pZ);

void    initSands(void);
bool    updateSands(void);
void    drawSands(int16_t y, uint8_t *pBuffer);
