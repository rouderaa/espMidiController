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
#include "MidiParser.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
MidiParser midiParser;

static const int MAXOUTBUFFER=128;
uint8_t outBuffer[MAXOUTBUFFER];

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

  midiParser.clear();

  Serial.println("MidiController activated");
}

// Usefull during debugging
static void printBuffer(char *label, uint8_t *toBuffer, int bufferLength) {
  Serial.printf("%s", label);
  for(int index = 0; index < bufferLength; index++)
    Serial.printf("%02X", toBuffer[index]);
  Serial.printf("\n\r");
}

void loop() {
  // MidiParser reads from Serial2 into its buffer
  midiParser.loop();
  if (midiParser.getLen() > 0) {
    SerialBT.write(midiParser.getBuffer(), midiParser.getLen());
    printBuffer("", midiParser.getBuffer(), midiParser.getLen());
    midiParser.clear();
  }

  // Midi stream from Bluetooth to controller
  int nrBytesToWrite = SerialBT.available();
  if (nrBytesToWrite > 0) {
    if (nrBytesToWrite >= MAXOUTBUFFER)
      nrBytesToWrite = MAXOUTBUFFER;
    size_t nrBytesWritten = SerialBT.readBytes(outBuffer, nrBytesToWrite);
    // printBuffer((char*) "O", outBuffer, nrBytesRead);
    Serial2.write(outBuffer, nrBytesWritten);
  }
}