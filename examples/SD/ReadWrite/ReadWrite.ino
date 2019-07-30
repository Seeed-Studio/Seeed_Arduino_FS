/*
  SD card read/write

 This example shows how to read and write data to and from an SD card file
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#ifdef KENDRYTE_K210
  #include <SPIClass.h>
#else
  #include <SPI.h>
#endif
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
#define SERIAL Serial

#define csPin 4
#ifdef ARDUINO_ARCH_SAMD
#undef SERIAL Serial
#define SERIAL SerialUSB
#endif

File myFile;

void setup() {
  // Open SERIAL communications and wait for port to open:
  SERIAL.begin(115200);
  while (!SERIAL) {
    ; // wait for SERIAL port to connect. Needed for native USB port only
  }


  SERIAL.print("Initializing SD card...");

  if (!SD.begin(csPin)) {
    SERIAL.println("initialization failed!");
    while (1);
  }
  SERIAL.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    SERIAL.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    SERIAL.println("done.");
  } else {
    // if the file didn't open, print an error:
    SERIAL.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    SERIAL.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      SERIAL.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    SERIAL.println("error opening test.txt");
  }
}

void loop() {
  // nothing happens after setup
}


