# r307 Fingerprint Sensor Module on ESP32
The Following Code is a Library for interfacing r307 Fingerprint Module with ESP32 on Embedded C Language

# GPIO Functions:
| ESP32 Devkit v1   | r307 Fingerprint Sensor |
| ------------- | ------------- |
| VIN pin | Pin 1 |
| GND pin | Pin 2 |
| GPIO 16 | Pin 3 |
| GPIO 17 | Pin 4 |

* Pin 5 and Pin 6 of the sensor can be opened ( No Connection )

# Connection Diagram:
![ESP32_R307_Connection](https://user-images.githubusercontent.com/99990377/171999044-11c50e19-c3a8-41ce-922c-179af355bffc.png)

# Understanding the Flow:
* This code is developed for ESP32 on Embedded C Language using FreeRTOS.
* There are 2 files you need to import which are going to be the library for interfacing ESP32 with R307 Sensor Module.
* R307 Fingerprint Sensor by default runs on UART Baud : **57600, with 8 data bits, 1 stop bit and no partiy.**
* Once **"r307_init()"** is called, that shall initialize UART ESP32 with the set parameters.
* Further there are 3 sections for the following library :
  * check_sum()
  * r307_reponse()
  * r307_response_parser()
* The **check_sum()** performs checksum modulo 256 on **Package Identifier, Package Length, Instruction Code and ( if used ) Packet Data** like new address, new password, etc.
* **r307_reponse()** function is responsible to receive package responses sent via the sensor module to ESP32.
* Lastly, **r307_response_parser()** function has the prime role of parsing every response received from the fingerprint sensor.
* There are several functions involved, total 22 for this library currently, that perform various tasks like setting new module address & new module password, reading system parameters, capturing or verifying or storing finger, etc.
* All these functions are written as per their names given in the user manual for r307 fingerprint module.
* Last but not the least, this repo is a library and doesn't have any example codes yet although every component you need to build a program for yourself can be easily done as comments and briefing is done for every code of line used.
* Please note, everytime you use any function to perform a task, you will have to provide the 32-bits Module address ( Default Address : 0xFF, 0xFF, 0xFF, 0xFF & Default Password : 0x00, 0x00, 0x00, 0x00 )
* Also note that any extra packet data if being used has to be declared in an char array with hex values as the data.

# Conclusion:
* Interfacing ESP32 with r307 Fingerprint Sensor wasn't the easiest or the most difficult but I did encountered lot of odds to develop the library on C as whole of the internet happens to use Arduino IDE.
* Examples codes are not yet available, but if I work on them in future then will surely commit the same.
* There are two functions missing : "WriteNotepad" & "ReadNotepad". I tried but couldn't crack them, so will commit the same whenever possible in future.
* Do share with others and I hope you all like it :-D

