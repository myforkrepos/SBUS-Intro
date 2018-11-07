# SBUS Introduction
SBUS is a relatively new protocol for sending position commands to servos. Unlike the standard [PWM](https://github.com/bolderflight/PWM-Intro), SBUS enables up to 16 servos to be bussed off of a single cable! This capability often facilitates much cleaner wiring for your robot or drone. In this introductory tutorial, we will explore what SBUS is and then practice what we've learned by commanding some servos to move! This tutorial will use a [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) along with the [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/).

# Necessary Hardware
   * [Teensy 3.2](https://www.pjrc.com/store/teensy32.html)
   * [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/)
   * SBUS capable servos
   * Voltage regulator to power the servos. Typically servos are powered with 5V DC; however, they can draw a lot of current! I recommend using either: a 5V DC power supply capable of providing at least 2A of current (i.e. a commonly found wall wart power supply may work) or an ESC with a battery eliminator circuit (BEC).

# SBUS
SBUS is a digital serial data bus created by [Futaba](https://www.futabarc.com/sbus/), which enables a single cable to carry the signals for up to 16 servos. With SBUS, each servo is programmed with a unique channel number and responds only to commands sent to that channel.

The SBUS protocol uses inverted serial logic with a baud rate of 100000, 8 data bits, even parity bit, and 2 stop bits. An SBUS packet is 25 bytes long consisting of:
   * Byte[0]: SBUS Header, 0x0F to identify the start of the packet
   * Byte[1-22]: 16 servo channels, 11 bits per channel
   * Byte[23]:
      * Bit 7: digital channel 17 (0x80)
      * Bit 6: digital channel 18 (0x40)
      * Bit 5: frame lost (0x20)
      * Bit 4: failsafe activated (0x10)
      * Bit 0-3: n/a
   * Byte[24]: SBUS Footer, 0x00 (optionally used for telemetry by Futaba)

Futaba and FrSky are common manufacturers of SBUS capable transmitters, receivers, and servos. An SBUS packet is sent once 7-14 ms. Specifically, once every 7 ms for Futaba high speed, 14 ms for Futaba normal speed and 9 ms for FrSky.

By default, FrSky products set to 100% range will output SBUS servo channel values between 172 and 1811. Using FrSky's extended limits, you can take advantage of the full 0 - 2047 range available. 

The SBUS frame lost bit is sent by the receiver every time a frame is not received from the transmitter. After enough lost frames in a row occur, the receiver switches to failsafe mode and the failsafe activated bit is set. The SBUS packet also includes provisions for two digital channels.

One burgeoning market for SBUS is in flight controllers. Flight controllers are simply micro-controllers with specialized software and we'll build our own in a future tutorial using the [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) and [Teensy Backpacks](http://bolderflight.com/products/teensy/). SBUS provides a convenient means of measuring commands from the pilot by reading the SBUS packets from an SBUS receiver wirelessly connected to a pilot's transmitter. With this approach we're able to measure 16 different stick positions and transmitter switches using only 1 pin on our microcontroller! 

Contrast this with trying to acheive the same results using a [PWM](https://github.com/bolderflight/PWM-Intro) receiver - we would need to precisely measure pulse width for each channel of receiver data we would like to use as input and we would require a digital pin for each. That's a lot of pins!

Similarly, our flight controller can send commands to 16 SBUS servos using a single pin. So with SBUS, we could potentially have 16 inputs from our receiver and 16 outputs to SBUS capable servos using just a pair of pins (PWM would take 32 to acheive the same capability).

Furthermore, due to the nature of SBUS as a serial communication protocol, our microcontroller can easily and efficiently read and write packets and our SBUS packets are less suceptible to electrical noise than PWM.

Enough extolling the virtues of SBUS, let's start putting our knowledge to practice.

# Wiring
For this tutorial, we will be using a [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) and the [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/).

First, plug your [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) into the [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/) with the USB connector on the end of the backpack that has a white dot; this dot marks the location of the Teensy ground pin. 

Servos have 3 wires: a signal wire carrying the SBUS signal, a power wire supplying power for the servo motor to move, and a ground wire. Typically, the signal wire is white, orange, or yellow. Ground is typically black or brown. Power is always red. Plug your servos into the [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/) with the signal wire up. The power wire should be in the middle with the ground at the bottom.

<img src="/images/closeup.JPG" alt="closeup" width="500">

The [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/) buses all of the servo power and grounds together, so simply connect your voltage regulator power and ground to an unused channel. For this tutorial, we will not be using the SBUS RX capability of the [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/), so either make sure the solderpad is connected to VDD instead of 5V and TX instead of RX or do not connect anything to that pin.

<img src="/images/setup.JPG" alt="setup" width="500">

You'll want to set the assigned channel number for each servo. You can use an SBUS Servo Channel Changer to do this.

<img src="/images/channel.jpg" alt="channel" width="500">

# Software
We'll use the [SBUS library](https://github.com/bolderflight/SBUS) from Bolder Flight Systems for this tutorial. This library contains methods for reading and writing SBUS packets using either raw counts (i.e. 172 - 1811) or normalized values (i.e. +/- 1).

## Installation
Simply clone or download the [SBUS library](https://github.com/bolderflight/SBUS) to your Arduino libraries folder. For this tutorial, you can clone or download the code in this [repository](https://github.com/bolderflight/SBUS-Intro) and follow along or code it yourself.

## Tutorial
### Goals
The goal of this tutorial is to create a periodic function to command our servos to move across their entire range. We will use this goal to learn about SBUS and using it with the [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) and [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/).

### Code Walkthrough
#### Constants
First, we're going to code some useful constants and make them available globally by placing them outside of the _setup_ and _loop_ functions at the start of the file.

We declare an SBUS object which uses Serial2. Using the [Backpack pinout](http://bolderflight.com/products/teensy/pinout/), we can see that Serial2 has been reserved for the [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/).

```C++
SBUS sbus(Serial2);
```

Next, we create an array of 16 commands, one for each available SBUS channel.

```C++
float SbusCmds[16];
```

One of the easiest ways to create a periodic command is by generating a sine wave and for this we'll need a good source of time. In this case, we're using a millisecond accuracy timer and [elapsedMillis](https://www.pjrc.com/teensy/td_timing_elaspedMillis.html) for convenience; although, using millis would have worked as well.

```C++
elapsedMillis time_ms;
```

We need to send commands every 7-14 ms, so we'll create a microsecond accuracy timer to do that.

```C++
elapsedMicros time_us;
```

#### Setup
We'll print the command to the Serial Monitor so that we can see how it's changing, so we better start the USB Serial communications.

```C++
Serial.begin(115200);
while (!Serial && time_ms < 5000) {}
```

Now, we'll start the SBUS communication.

```C++
sbus.begin();
```

#### Loop
The first item of business is timing. We're going to send the SBUS commands to the servos every 10 ms, so we check to see if 10 ms has elapsed, if it has we'll zero the timer before computing and sending the commands.

```C++
  if (time_us > 10000) {
    time_us -= 10000;
```
Next, we need to compute the sine wave. In this case we're using a 1 Hz sine wave.

```C++
float Cmd = sinf(2.0f * M_PI * time_ms / 1000.0f);
```
The sine wave will give us a result in the range of -1 to +1. Because we'll be using the _writeCal_ method of writing the servo commands, which expects a command in the +/- 1 range, this will work directly for setting the command. So we'll step through each channel and set it to the current value from the sine wave.

```C++
for (unsigned int i = 0; i < sizeof(SbusCmds) / sizeof(float); ++i) {
  SbusCmds[i] = Cmd;
}
```

Finally, we'll send the commands to the servos.

```C++
sbus.writeCal(&SbusCmds[0]);
```

Now we'll print the command to the Serial Monitor so we can read it.

```C++
Serial.println(Cmd);
```

Done! Congrats, your code should compile and be ready to upload to the [Teensy 3.2](https://www.pjrc.com/store/teensy32.html).

### Experiment
Upload your code to the Teensy and apply servo power. You should see your servos move through their full range of motion repeating every second. Try to compare the servo arm position with the command being printed to the Serial Monitor. Most servos will move through almost 180 degrees of motion.

You can find a video of the results here:

[![Experiment Video](https://img.youtube.com/vi/j7PGzULhdo0/0.jpg)](https://www.youtube.com/watch?v=j7PGzULhdo0)

# Wrap Up
In this tutorial we learned about SBUS, which is a relatively new protocol for sending position commands to servos. We used a [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) and the [Teensy SBUS Backpack](http://bolderflight.com/products/teensy/sbus/) to command servos to move to positions computed using a sine wave.

# Further Reading
1. [Bolder Flight Systems SBUS library](https://github.com/bolderflight/SBUS): study the additional methods available and try them out for yourself!

# Next Steps
Armed with the knowledge and experience that you gained with this tutorial, see if you can do the following:
1. Change the sine wave frequency. How would you make it faster? Slower?
2. Command different periodic functions. Can you make the servos step to a position at a given time? Step back?
3. Write a program that allows you to enter an SBUS in the Serial Monitor and command the servos to move to that position.
4. Try tracing the full range of the servo arm on a piece of paper and estimate its angle. Send the servo an SBUS command, mark the resulting position and estimate its angle. Do this for several different SBUS commands throughout the full range. Can you estimate a function to convert angles to SBUS commands? Try writing a function that will take an angle command as a parameter, convert the angle to an appropriate SBUS command, and send that command to the servo.

# Next Tutorial
1. [Reading SBUS](https://github.com/bolderflight/Reading-SBUS): Gain experience reading SBUS packets!
2. [SBUS to PWM Converter](https://github.com/bolderflight/SBUS-to-PWM): Use your knowledge of PWM and SBUS to create your own SBUS to PWM converter!
