This is a libary for arduino connecting to a Soladin 600 inverter
Soladin 600 en Windmaster 500 inverters from mastervolt are basicly the same
Connecting to windmaster is not tested.

This library is a modified version of the original by git use teding.
SoftwareSerial is replaced by the stream class so physical serial port can be used. 
Also the serial wait state and timeouts were modified.

This lib was tested with Arduino 1.6.1 for ESP8266.

Hardware connection:

	Inside soladin 600 are two CNY17 Opto's


      RJ 12 Connector	        1 nu              socket(soladin)
      1  2  3  4  5  6	        2 TX -            6  5  4  3  2  1
   ____________________	        3 TX +         ____________________
   | || || || || || || |        4 RX +	       | || || || || || || |
   | || || || || || || |        5 RX -         |                   |
   |                   |        6 nu           |                   |
   |_____frontview_____|                       |_____frontview____ |
         |_______|                                    [______]
           |___| 


		   
         ESP8266          ____     RJ 12 connector       
   + 3.3v      -----------[____]--------- 3  (TX+)             
                          150R                               
    (soft)TX ---------------------------- 2  (TX-)	
                         ____
    + 3.3V     ----------[____]-----†---- 4  (RX+)	
                          10K      |
    (soft)RX ---------------------†

     gnd     ---------------------------- 3  (RX-)	
   


Retrieved from "http://wiki.firestorm.cx/index.php/Soladin"
   
Serial Protocol:
 
Every packet has the following common fields: 

The first 4 bytes contain the packets source and destination: 

0x0000: 16 bit destination address for the packet.
0x0000: 16 bit source address for the packet.
0x0000: 16 bit command ID. 

The master device (the computer) uses address 0x0000. In the probe command from the Windows software, 
the source and destination addresses are both set to 0x0000. 
This may indicate the packet is a broadcast packet intended for all non-master devices on the bus. 
All data is transmitted as big-endian (most significant byte first). 

0x00: The last byte of every packet is a checksum. It is the lower 8 bits of the sum of all the previous bytes in the packet. 

I've included the responses to each command from my Soladin 600, address 0x0011. This appears to be the same address for all Soladin 600's. 

0xC1: Probe 
The first packet transmitted seems to be a probe command. Note that the source and destination addresses are both set to 0x0000. 

TX: 00 00 00 00 C1 00 00 00 C1
RX: 00 00 11 00 C1 F3 00 00 C5

Other than the device address, the response from my Soladin 600 doesn't appear to contain any information. 

In cases where there are more than once device, will they all respond to this command? 
Can a devices address be changed? 

0xB4: Firmware Information 
Returns version information about the firmware. 

TX: 11 00 00 00 B4 00 00 00 C5
RX: 00 00 11 00 B4 F3 00 00 00 00 00 00 00 E3 00 04 01 34 06 00 00 00 00 00 00 00 00 00 00 00 DA

0xE3: Firmware ID
0x0104: Firmware version (1.04)
0x0634: Firmware date


0xB6: Device Stats 
Returns statistics about the operation of the device. 

TX: 11 00 00 00 B6 00 00 00 C7
RX: 00 00 11 00 B6 F3 00 00 04 03 35 00 8A 13 F4 00 00 00 24 00 90 0B 00 1F DB BC 01 00 00 00 FD

0x06: 0x0000: Flags, normal operation in the response above
0x08: 0x0304: PV voltage * 10 = 77.2 V in the response above
0x0A: 0x0035: PV amperage * 100 = 0.53 A
0x0C: 0x138A: Grid frequency * 100 = 50.02 Hz
0x0E: 0x00F4: Grid voltage = 224 V
0x10: 0x0000: Unknown
0x12: 0x0024: Grid power output = 36 W
0x14: 0x000B90: Total grid power output * 100 = 29.60 kWh
0x17: 0x1F: Device temperature = 31 °C
0x18: 0x0001BCDB: Total operating time = 113883 minutes
0x1C: 0x0000: Unknown

Flags

Flags are bit mapped and represent current status of the inverter. Normal opartion of the inverter is 
identified with no flag being set.

0x0001: Usolar too high
0x0002: Usolar too low
0x0004: No Grid
0x0008: Uac too high
0x0010: Uac too low
0x0020: Fac too high
0x0040: Fac too low
0x0080: Temperature too high
0x0100: Hardware failure
0x0200: Starting
0x0400: Max power
0x0800: Max current

0xB9: Read Maximum Power 
Returns maximum output power of the inverter.

TX: 11 00 00 00 B9 00 00 CA
RX: 00 00 11 00 B9 F3 00 00 20 00 00 00 1B 00 21 00 22 00 00 00 E5 02 7E 48 36 00 00 00 00 00 1E

0x18: 0x0036: Maximum power = 54 W

0x97: Reset Maximum Power

Resets maximum uotput power returned by command 0xB9

TX: 11 00 00 00 97 01 00 00 A9
RX: 00 00 11 00 97 01 00 00 A9

0x9A: History Data

The inverter stores data (grid energy and time) for last 10 day. The inverter has no clock built-in, a
day is therfore defined as operating season.

TX: 11 00 00 00 9A 00 00 AB

0x05: 0x00 Day to read. 0 - today, 9 - 9 days before today

RX: 00 00 11 00 9A 54 05 04

0x05: 0x54: Daily opertion time * 5 min = 420 minutes
0x06: 0x0005: Grid output * 100 = 0.05 kWh
