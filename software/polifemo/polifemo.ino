/*
 * Epoca Polifemo
 * Copyright (C) 2016 Nicola Corna <nicola@corna.info>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <avr/sfr_defs.h>
#include <avr/wdt.h>
#include <stdint.h>

#include <TaskScheduler.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "MedianFilter.h"
#include "MovingAverage.h"

const uint8_t onCommand = A1;   // PC1 (A1 = 15)
const uint8_t buttonLedPWM = 3; // PD3, OC2B
const uint8_t buttonSense = 2;  // PD2, INT0
const uint8_t sigBar0 = 9;      // PB1, OC1A
const uint8_t sigBar1 = 10;     // PB2, OC1B
const uint8_t ds18b20 = 5;      // PD5 (OC0B)

const unsigned long totalTime = 20000;
const uint16_t refreshPeriod = 120;

const uint8_t watchDogTimeout = WDTO_500MS; //Make sure that watchDogTimeout >> refreshPeriod

const uint16_t dimUpPeriod = 5;
const uint8_t dimUpStep = 2;    //The total dimUp time is dimUpPeriod * (255 / dimUpStep)

const uint16_t dimDownPeriod = 10;
const uint8_t dimDownStep = 2;  //The total dimDown time is dimDownPeriod * (255 / dimDownStep)

/*
const uint16_t distanceMin = 20;
const uint16_t distanceMax = 765;
*/

//30 cm per step
const uint16_t distanceMin = 20;
const uint16_t distanceMax = 600;

//2 °C per step
const int16_t temperatureMin = -3 * 128;
const int16_t temperatureMax = 35 * 128;

//The maximum input value for the LED bars is 1.25V: we have to scale the maximum PWM in order to match it
const uint8_t sigBarMaxPWM = 115;

int16_t ledBrightness = 0;
bool buttonPressed = false;

OneWire oneWire(ds18b20);
DallasTemperature temperatureBus(&oneWire);
DeviceAddress temperatureAddress;

int16_t temperature = 20 * 128;  // 1/128 °C
uint16_t distance = 20;   // cm

MedianFilter<uint16_t, 3> distanceMedian(0xFFFF, distance);
MovingAverage<uint16_t, 5> distanceAverage(distance);

String rawRx = "";

//Forward declarations
void powerButtonPressed();
void displayData();
void updateAndDisplayData();
void dimUp();
void dimDown();
void powerOffSequence();

Task updateAndDisplayDataTask(refreshPeriod, TASK_FOREVER, &updateAndDisplayData);
Task dimUpTask(dimUpPeriod, TASK_FOREVER, &dimUp);
Task dimDownTask(dimDownPeriod, TASK_FOREVER, &dimDown);
Task powerOffSequenceTask(totalTime, TASK_ONCE, &powerOffSequence);

Scheduler runner;

//Disable the watchdog as soon as possible (we don't want the watchdog during the initialization phase)
//see http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));
void get_mcusr(void)
{
  MCUSR = 0;
  wdt_disable();
}

void setup() {
  //Set the direction and the default value of all the pins
  pinMode(onCommand, OUTPUT);
  digitalWrite(onCommand, HIGH);

  pinMode(buttonLedPWM, OUTPUT);
  analogWrite(buttonLedPWM, ledBrightness);

  pinMode(buttonSense, INPUT);
  //Not optimal: if the user keeps the button pressed the interrupt is fired only once (but who cares??)
  attachInterrupt(digitalPinToInterrupt(buttonSense), powerButtonPressed, CHANGE);

  pinMode(sigBar0, OUTPUT);
  pinMode(sigBar1, OUTPUT);

  displayData();

  //We're throwing the sigBarN outputs in a 1st order LPF, therefore the higher the frequency of the PWM, the more stable the response
  //Here's we're setting the prescaler equal to 1 and the Fast-PWM mode (PWM frequency = 43.2 kHz)
  //This interferes with the Servo library
  TCCR1B |= _BV(CS10) | _BV(WGM12);
  TCCR1B &= ~( _BV(CS11) | _BV(CS12) );

  Serial.begin(9600);

  temperatureBus.begin();
  if (temperatureBus.getAddress(temperatureAddress, 0))
  {
    temperatureBus.setWaitForConversion(false);
    temperatureBus.setResolution(9);
    temperatureBus.requestTemperatures();
  }

  pinMode(LED_BUILTIN, OUTPUT);
  delay(100); //Now both the measurements should be ready

  runner.addTask(updateAndDisplayDataTask);
  runner.addTask(dimUpTask);
  runner.addTask(dimDownTask);
  runner.addTask(powerOffSequenceTask);

  updateAndDisplayDataTask.enableDelayed();
  dimUpTask.enableDelayed();
  powerOffSequenceTask.enableDelayed();
  
  wdt_reset();
  wdt_enable(watchDogTimeout);  //Enable the watchdog; see dimDown
}

void loop() {
  runner.execute();

  if (buttonPressed) {
    buttonPressed = false;
    powerOffSequenceTask.restartDelayed();  //Restart the poweroff timer
    dimDownTask.disable();                  //Remove an eventual dimDown
  
    if (ledBrightness != 255)
      dimUpTask.enableDelayed();  //Increase the LED's brightness if they're not at 100%
    }
}

void powerButtonPressed() {
  buttonPressed = true;
}

void updateAndDisplayData() {
  static String rawRx;
  int8_t start;
  int8_t end;
  char buffer[65];
  int16_t rawTemperature;
  
  wdt_reset();

  //Temperature
  rawTemperature = temperatureBus.getTemp(temperatureAddress);
  if (rawTemperature > DEVICE_DISCONNECTED_RAW)
    temperature = constrain(rawTemperature, temperatureMin, temperatureMax);

  temperatureBus.requestTemperaturesByAddress(temperatureAddress);
  
  //Distance
  end = Serial.available();
  Serial.readBytes(buffer, end);
  buffer[end] = '\0';

  rawRx += buffer;
  end = rawRx.lastIndexOf('\r');
  
  if (end > 0) {
    start = rawRx.lastIndexOf('R', end);

    if (start >= 0) {
      uint16_t unfilteredDistance = rawRx.substring(start + 1, end).toInt();
      
      if (unfilteredDistance != 0) {
        distance = distanceAverage.filter(distanceMedian.filter(constrain(unfilteredDistance, distanceMin, distanceMax)));
      }
      
      rawRx.remove(0, end + 1); //Throw away older data
    }
  }

  displayData();

  wdt_reset();
}

void displayData() {
  static const uint8_t sigBarStep = sigBarMaxPWM / 19;
  static const uint8_t sigBarStep_2 = sigBarStep / 2;

  uint8_t value;

  //Find the maximum PWM value, a combination of ledBrightness (0->255) and sigBarMaxPWM(0->255)
  //For an explaination of that "+ 1" see this link http://forum.arduino.cc/index.php/topic,46546.0.html
  const uint8_t maxPWM = map(ledBrightness, 0, 255 + 1, 0, sigBarMaxPWM + 1);

  //Then map the distance (distanceMin->distanceMax) to the new PWM range(0->maxPWM)
  value = map(distance, distanceMin, distanceMax + 1, 0, maxPWM + 1);

  //Then approximate the final value to one of the 20 steps
  value = (value + sigBarStep_2) / sigBarStep * sigBarStep;
  analogWrite(sigBar1, value);

  value =  map(temperature, temperatureMin, temperatureMax + 1, 0, maxPWM + 1);
  value = (value + sigBarStep_2) / sigBarStep * sigBarStep;
  analogWrite(sigBar0, value);
}

void dimUp() {
  ledBrightness = constrain(ledBrightness + dimUpStep, 0, 255);
  analogWrite(buttonLedPWM, ledBrightness);
  displayData();

  if (ledBrightness == 255) //Maximum brightness reached
    dimUpTask.disable();
}

void dimDown() {
  ledBrightness = constrain(ledBrightness - dimDownStep, 0, 255);
  analogWrite(buttonLedPWM, ledBrightness);
  displayData();

  if (ledBrightness == 0) { //Minimum brightness reached
    digitalWrite(onCommand, LOW); //Power off everything

    wdt_reset();
    while(true) {    
      //Force a watchdog timeout if the AVR is still on after watchDogTimeout
      //This is to prevent dead-lock situations that can occur if the user press the button exactly
      //after digitalWrite(onCommand, LOW), as the capacitors keep the AVR powered for a while
    }
  }
}

void powerOffSequence() {
  dimDownTask.enableDelayed();  //Start the "dimDown" countdown
}

