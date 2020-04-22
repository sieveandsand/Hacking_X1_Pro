# Hacking X1 Pro
This is a documentation of my attempt at reverse engineering the Exway X1 Pro electric skateboard remote &amp; board.

## Overview

The ultimate goal of this project is to sniff the communication between the skateboard remote and the board and to reverse engineer the packets being transmitted between them. Once the packets are deciphered, I am looking to make a wireless breaking light that works by sniffing the packet data.

Meanwhile, this project will take some detour and will first be examining the hardware used by the remote.

## Remote Hardware

![IMG_4455](/Photos/IMG_4455.jpg)

Upon opening up the casing of the remote, we are greeted with a Cortex M3 microcontroller which is tucked under a small display. Vibration motor and pushbutton can be seen on the right side of the PCB. Antenna is located on the bottom on the board.

![IMG_4456](Photos/IMG_4456.jpg)

A closer look at the ARM chip reveals its chip module: [GD32F130](http://gd32mcu.21ic.com/data/documents/shujushouce/GD32F130xx_Datasheet_Rev3.1.pdf) This chip is manufactured by GigaDevice and has same pinout as [STM32L071xx LQFP48](https://www.st.com/resource/en/datasheet/stm32l071cz.pdf). GD32F130 is basically a clone of the ST chip and carries the same functionality as LQFP48.

![IMG_4458](Photos/IMG_4458.jpg)

The back of the remote PCB reveals two more ICs. The one on the right is RFX2401C RF front-end module. The one on the left is Si24R1 and is a Chinese clone of nRF24L01P.



## Finding the Right Frequency

Knowing the only fact that Si24R1 is a 2.4GHz ISM chip is not enough for sniffing, we need more specific information on the number of channels, what frequencies these channels are operating on, and the bit rate data are transferred on. Luckily all these information can be found in the FCC database. X1 Pro's FCC ID is [2APTF-X1](https://fccid.io/2APTF-X1), and its test report submitted to the FCC contains the information we wanted.

From the test report, we can see that X1 Pro operates between 2402-2480 MHz and has a total of 40 different channels. Each channel is separated by a 2 MHz frequency offset.

> | Channel | Frequency (MHz) | Channel | Frequency (MHz) | Channel | Frequency (MHz) |
> | ------- | --------------- | ------- | --------------- | ------- | --------------- |
> | 01      | 2402            | 11      | 2422            | 21      | 2442            |
> | 02      | 2404            | 12      | 2424            | 22      | 2444            |
> | 03      | 2406            | 13      | 2426            | 23      | 2446            |
> | ~       |                 | ~       |                 | ~       |                 |
> | 09      | 2418            | 19      | 2438            | 39      | 2478            |
> | 10      | 2420            | 20      | 2440            | 40      | 2480            |

Know the fact that each channel is separated by 2 MHz offset is crucial in determining the air data rate the chip is operating on. In the below snippet of nRF24L01+ datasheet (Si24R1 is perfectly compatible with nRF24L01+), it specifies that a 2MHz channel spacing translates to 2Mbps air data rate.

![data_sheet](Photos/NRF24L01_datasheet_screenshot.jpg)



## Promiscuous Sniffing on nRF24L01+

Given the fact that Si24Ri is a perfect clone of nRF24L01+, and nRF24L01+ is well documented, I decided to look around and see if there are any available methods of promiscuously sniffing nRF24L01+ packets. It turns out nRF24L01+ has been successfully sniffed in 2011. Travis Godspeed did a well-documented [blog](http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html) on technical implementations of this promiscuous sniffing. My sniffing program for this project is based on his implementation, and I recommend anyone who is interested in this project to take a look at his writeup.

Travis' implementation of promiscuous sniffing can be boiled down to 4 parts:

1. Limit the MAC address to 2 bytes
2. Disable checksums
3. Set the MAC to be the same as the preamble
4. Sort received noise for valid MAC address

### Hardware Setup