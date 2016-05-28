# Aviation-Altimeter
Barometric altimeter with MEMS pressure sensor, ARM processor and OLED display

Aviation use altimeters are extremely sensitive mechanical devices which use a bellows to convert pressure to displacement, and then use a clockwork mechanism to convert the displacement to dial position on the display. Since the overall displacement is quite small (a few millimeters for the ~20,000 ft altitude range of the device, the mechanism must be fairly complex, precise and maintained periodically. Accurate and sensitive MEMS sensors have been available since around 2005, which are a perfect replacement for the mechanical altimeters. However the early versions were analog (simple Wheatstone bridges) which meant that they were susceptible to drifts, required temperature compensation and fairly complex electronics to read the output voltage with sufficient precision to allow use as an aviation altimeter.

Technology to the rescue, recently these problems were solved with tiny pressure sensors which include on board temperature compensation, low drift, and high precision analog digital converters on-board. The user uses simple communication protocol to interface with them, and it is only a matter of following the datasheet to implement up to 24 bit altitude readout. Quite incredible, where the tecnology has come!

One such notable pressure sensor is Measurement Specialities MS5611 series of pressure sensors. They have 24 bit precision and high accuracy, with factory calibrated coefficients which are used to convert raw measurement values to pressure reading. It is a dream come true; such a precision and accuracy is difficult to come by at such low prices, of about $8~10.Better, they come installed in many droneflight controllers. This is great because for about $25, you get a STM32F103 microcontroller with many other sensors, clock, regulator, all broken out to .1" pin headers. A great platform to experiment and develop with.

In this project, a full function barometric altimeter was implemented using a Flip32 (Naze32 clone) flight controller board with installed MS5611 pressure sensor, an SSD1306 graphic OLED screen on I2C, and a rotary encoder used in consumer products for volume etc. setting. A single LiPo cell powers the system, with about 40mA current draw.

There is QNH compensation, and it is possible to learn the local QNH value from a nearby airport, dial it into the device, and obtain the MSL altitude in feet. In exactly the same way as a conventional altimeter is used.

The project requires u8glib ARM port, and compiled with GCC ARM Embedded toolchain, easily downloadable from Launchpad. 


