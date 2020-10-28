## Overview

If you are reading this, then I am sure you are one cool electric skateboarder. I absolutely love electric skateboarding, but what I don't like is that all the cool accessories are proprietary and expensive. Sniffer class provides telemetry data for electric skateboards (currently only Exway). It promiscuously sniffs traffic between the remote and the board. You can get access to these data by calling getDirection(), getGear(), and getThrottle().



The wiki document contains more information on the sniffer implementation. Otherwise, stay safe and happy carving!



## Notes

- Previous README.md has moved to wiki.md
- Sniffer only supports Arduino at the moment
- Sniffer only works with Exway board (which is the only board I have access to)
- Currently working on:
  - Improving the performance of getDirection(), getThrottle(), and getGear() functions



## Hardware Requirement

- Arduino or other Arduino compatible hardware

- nRF24L01+ transceiver

  

## Installation

### For Arduino Users

1. Download the RF24 library from https://github.com/nRF24/RF24.

   ```bash
   git clone https://github.com/nRF24/RF24
   ```

2. Download this repository.

   ```bash
   git clone https://github.com/sieveandsand/Hacking_X1_Pro
   ```

3. In the Arduino IDE, add RF24 folder and project_sniffer folder from Hacking_x1_Pro to Arduino library.

   Folders can be added to Arduino library by "Sketch" --> "Include library" --> "Add .zip library".

4. Start coding your own sketch or try out the example Arduino sketch in the "examples/sniffer_example" folder.



## Quick Reference

### For Exway boards

- Use SNIFFER(CE, CSN) constructor. CE, CSN are pin numbers of the nRF24 chip.

- Following functions must be called before using getDirection(), getGear(), and getThrottle()

  ```c++
  SNIFFER::init();
  SNIFFER::scanWithMagicID();
  ```

- Make sure throttle wheel on the remote is in neutral position when scanWithMagicID() is running.