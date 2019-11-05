# Hacking X1 Pro
This is a documentation of my attempt at reverse engineering the Exway X1 Pro electric skateboard remote &amp; board.

## Overview

The ultimate goal of this project is to sniff the communication between the skateboard remote and the board and to reverse engineer the packets being transmitted between them. Once the packets are deciphered, I am looking to make a wireless breaking light that works by sniffing the packet data.

Meanwhile, this project will take some detour and will first be examining the hardware used by the remote.

## Remote Hardware

![IMG_4455](C:\Users\qiaof\repos\Hacking_X1_Pro\Remote PCB Photo\IMG_4455.jpg)

Upon opening up the casing of the remote, we are greeted with a Cortex M3 microcontroller which is tucked under a small display. Vibration motor and pushbutton can be seen on the right side of the PCB. Antenna is located on the bottom on the board.

![IMG_4456](C:\Users\qiaof\repos\Hacking_X1_Pro\Remote PCB Photo\IMG_4456.jpg)

A closer look at the ARM chip reveals its chip module: [GD32F130](http://gd32mcu.21ic.com/data/documents/shujushouce/GD32F130xx_Datasheet_Rev3.1.pdf) This chip is manufactured by GigaDevice and has same pinout as [STM32L071xx LQFP48](https://www.st.com/resource/en/datasheet/stm32l071cz.pdf). GD32F130 is basically a clone of the ST chip and carries the same functionality as LQFP48.

![IMG_4458](C:\Users\qiaof\repos\Hacking_X1_Pro\Remote PCB Photo\IMG_4458.jpg)

The back of the remote PCB reveals two more ICs. The one on the right is RFX2401C RF front-end module. The one on the left is Si24R1 and is a Chinese clone of nRF24L01P.