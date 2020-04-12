/*!
 *  @file       MidiParser.cpp
 *  Project     esp32MIDIBridge
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
#include <stdint.h>
#include "MidiParser.h"

MidiParser::MidiParser()
{
  midiMessage1.len = 0;
  midiMessage2.len = 0;

  toFillMidiMessage = &midiMessage1;
  toSendMidiMessage = &midiMessage2;
}

void MidiParser::swapBuffers()
{
  // Check if sendbuffer was transmitted
  if (toSendMidiMessage->len != 0)
  {
    Serial.printf("ERROR: BT sendbuffer overflow\n\r");
  }

  // Do the buffer swap
  if (toFillMidiMessage == &midiMessage1)
  {
    toFillMidiMessage = &midiMessage2;
    toSendMidiMessage = &midiMessage1;
  }
  else
  {
    toFillMidiMessage = &midiMessage1;
    toSendMidiMessage = &midiMessage2;
  }

  // Clear swapped fill buffer
  toFillMidiMessage->len = 0;
}

static unsigned long miditimeout = ULONG_MAX;
void MidiParser::loop()
{
  // Collect MIDI commands into a buffer
  if (Serial2.available())
  {
    uint8_t b = Serial2.read();
    if ((b == 0xf8) || (b == 0xfe))
      return; // IGNORE 

    if ((b & 0x80) != 0)
    {
      // Start of new command detected
      swapBuffers();
    }

    // Serial.printf("%02X", b);
    toFillMidiMessage->put(b);

    if ((b & 0x80) == 0)
    {
      // Continuation of command detected
      miditimeout = millis();
    }
  }
  else
  {
    // No new byte detected but timeout occures for current data
    if ((millis() > miditimeout) && (toFillMidiMessage->isFilled()))
    {
      swapBuffers();
    }
  }
}

bool MidiParser::available()
{
  return toSendMidiMessage->isFilled();
}

uint8_t *MidiParser::getBuffer()
{
  return toSendMidiMessage->data;
}

int MidiParser::getLen()
{
  return toSendMidiMessage->len;
}

void MidiParser::clear()
{
  toSendMidiMessage->len = 0;
}
