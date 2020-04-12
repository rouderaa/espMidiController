/*!
 *  @file       MidiParser.h
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

#pragma once

class MidiParser
{
    private:
    class MidiMessage
    {
    public:
        static const int MAXDATA = 128;
        uint8_t data[MAXDATA];
        int len;

        void put(uint8_t b)
        {
            if (len < (MAXDATA - 1))
            {
                data[len] = b;
                len++;
            }
            else
            {
                Serial.printf("ERROR: buffer overflow\n\r");
            }
        }
        bool isFilled() { return (len > 0);}
    };
    void swapBuffers();

public:
    MidiParser();
    void loop();
    bool available();

    uint8_t *getBuffer();
    int getLen();
    void clear();

private:
    MidiMessage midiMessage1;
    MidiMessage midiMessage2;

    MidiMessage *toFillMidiMessage;
    MidiMessage *toSendMidiMessage;
};