/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2018 Bolder Flight Systems
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
* and associated documentation files (the "Software"), to deal in the Software without restriction, 
* including without limitation the rights to use, copy, modify, merge, publish, distribute, 
* sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all copies or 
* substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* Description: this program will output a sine wave on all 16 SBUS channels for 
* testing the Teensy SBUS Backpack (http://bolderflight.com/products/teensy/sbus). 
* The sine amplitude will be set to make the servo travel across its entire range
* and the excitation frequency will be 1 Hz.
*/

#include "SBUS.h"

/* The Teensy SBUS Backpack interfaces on Serial2 */
SBUS sbus(Serial2);
/* 16 SBUS channels */
float SbusCmds[16];
/* Time, ms. Used for computing sine wave */
elapsedMillis time_ms;
/* Timer, us. Used for SBUS frame timing at 100 Hz */
elapsedMicros time_us;

void setup()
{
  /* Serial for printing the command */
  Serial.begin(115200);
  while (!Serial && time_ms < 5000) {}
  /* Starting SBUS communication */
  sbus.begin();
}

void loop()
{
  if (time_us > 10000) {
    time_us -= 10000;
    /* 1 Hz sine wave */
    float Cmd = sinf(2.0f * M_PI * time_ms / 1000.0f);
    /* Set each channel's command to the current sine wave value */
    for (unsigned int i = 0; i < sizeof(SbusCmds) / sizeof(float); ++i) {
      SbusCmds[i] = Cmd;
    }
    /* 
    * Write the SBUS commands to the servos, note that we're using the writeCal
    * method, which expects commands in the range of +/- 1. If we instead used the 
    * write method, which expects raw counts (typically 172 to 1811), we would need
    * to apply a scale factor and bias first.
    */
    sbus.writeCal(&SbusCmds[0]);
    /* Print command */
    Serial.println(Cmd);
  }
}