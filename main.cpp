/*!
 *  @file       main.cpp
 *  Project     esp32MidiController
 *  @brief      esp32 based MIDI wireless bridge.
 *  @author     Rob van der Ouderaa
 *  @date       10/04/2020
 *  @license    MIT - Copyright (c) 2020 Rob van der Ouderaa
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

// Directly play a note on the MIDI hardware, just for testing
void playNote(uint8_t pitch) {
  uint8_t midiOnMessage[] = { 0x90, pitch, 40 };
  Serial2.write(midiOnMessage, sizeof(midiOnMessage));
  delay(200);

  uint8_t midiOffMessage[] = { 0x80, pitch, 00 };
  Serial2.write(midiOffMessage, sizeof(midiOffMessage));
  delay(200);
}

void setup() {
  Serial.begin(115200);

  // SerialBT is the wireless link to the Linux PC sending/transmitting MIDI
  SerialBT.begin("ESP32MIDI"); //Bluetooth device name

  // Serial2 is the hardware MIDI port directly connected to this ESP32
  uint32_t config = 134217756U; // Set serial port parameters like stopbits etc.
  Serial.printf("Config:%08X\n\r", config); // For debugging

  // Rx=IO21, TX=IO18 pins
  Serial2.begin(31250, config, GPIO_NUM_21, GPIO_NUM_18, false);

  // Signature pings at startup
  playNote(0x60);
  playNote(0x60);

  Serial.println("MidiController activated");
}

// Usefull during debugging
static void printBuffer(char *label, uint8_t *toBuffer, int bufferLength) {
  Serial.printf("%s", label);
  for(int index = 0; index < bufferLength; index++)
    Serial.printf("%02X", toBuffer[index]);
}

const int MAXBUFFER=128;
uint8_t buffer[MAXBUFFER];

// Remove 0xFE and 0xF8 Midi messages from stream 
void filterBytes(uint8_t *toBuffer, size_t &nrOfBytes) {
  uint8_t *toDestBuffer = toBuffer;
  for(int index = 0; index < nrOfBytes; index++) {
    if ((*toBuffer != 0xFE) && (*toBuffer != 0xF8)) {
        *toDestBuffer = toBuffer[index];
        toDestBuffer++;
    }
  }
  nrOfBytes = toDestBuffer-toBuffer;
}

void loop() {
  int nrBytesToRead = Serial2.available();
  if (nrBytesToRead > 0) {
    if (nrBytesToRead >= MAXBUFFER)
      nrBytesToRead = MAXBUFFER;
    size_t nrBytesRead = Serial2.readBytes(buffer, nrBytesToRead);

    filterBytes(buffer, nrBytesRead);
    if (nrBytesRead > 0) {
      // printBuffer((char*) "I", buffer, nrBytesRead);
      SerialBT.write(buffer, nrBytesRead);
    }
  }
  nrBytesToRead = SerialBT.available();
  if (nrBytesToRead > 0) {
    if (nrBytesToRead >= MAXBUFFER)
      nrBytesToRead = MAXBUFFER;
    size_t nrBytesRead = SerialBT.readBytes(buffer, nrBytesToRead);
    // printBuffer((char*) "O", buffer, nrBytesRead);
    Serial2.write(buffer, nrBytesRead);
  }
}