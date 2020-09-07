// header file for main.cpp

#ifndef __MAIN_H__
#define __MAIN_H__

#include <iostream>
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "RF24-master/RF24.h"
#include "RF24-master/RF24_config.h"
#include "RF24-master/nRF24L01.h"

typedef enum {
    idle, scan, set
} State;


RF24* setupRadio();

#endif
