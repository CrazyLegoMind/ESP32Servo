/*
 * ESP32PWM.cpp
 *
 *  Created on: Sep 22, 2018
 *      Author: hephaestus
 */

#include <ESP32PWM.h>
#include "esp32-hal-ledc.h"
// initialize the class variable ServoCount
static int PWMCount = -1;              // the total number of attached servos
static ESP32PWM * ChannelUsed[NUM_PWM]; // used to track whether a channel is in service
// The ChannelUsed array elements are 0 if never used, 1 if in use, and -1 if used and disposed
// (i.e., available for reuse)

ESP32PWM::ESP32PWM() {
	resolutionBits=8;
	pwmChannel = -1;
	pin = -1;
}

ESP32PWM::~ESP32PWM() {
	// TODO Auto-generated destructor stub
}

void ESP32PWM::detach() {
	ChannelUsed[getChannel()] = NULL;
	pwmChannel = -1;
	pin = -1;
	attachedState = false;
}

void ESP32PWM::attach(int p) {
	pin = p;

	if (PWMCount == NUM_PWM) {
		return;
	}
	getChannel();
	attachedState = true;
}

int ESP32PWM::getChannel() {
	if (PWMCount == -1) {
		for (int i = 0; i < NUM_PWM; i++)
			ChannelUsed[i] = NULL; // load invalid data into the storage array of pin mapping
		PWMCount = PWM_BASE_INDEX; // 0th channel does not work with the PWM system

	}
	if (pwmChannel < 0) {
		for (int i = PWM_BASE_INDEX; i < NUM_PWM; i++) {
			if (ChannelUsed[i] == NULL) {
				ChannelUsed[i] = this;
				pwmChannel = i;
				PWMCount++;
				//Serial.println("PWM channel requested " + String(i));
				return pwmChannel;
			}
		}
		Serial.println(
				"ERROR All PWM channels requested! " + String(PWMCount));
	}
	return pwmChannel;
}

double ESP32PWM::setup(double freq, uint8_t resolution_bits) {
	resolutionBits = resolution_bits;
	if (attached()) {
		detachPin(pin);
		double val = ledcSetup(getChannel(), freq, resolution_bits);
		attachPin(pin);
		return val;
	}
	return ledcSetup(getChannel(), freq, resolution_bits);
}



float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void ESP32PWM::writeScaled(float duty) {
	write(mapf(duty, 0.0, 1.0, 0, (float) ((1 << resolutionBits) - 1)));
}
void ESP32PWM::write(uint32_t duty) {
	ledcWrite(getChannel(), duty);
}
double ESP32PWM::writeTone(double freq) {
	resolutionBits=10;
	return ledcWriteTone(getChannel(), freq);
}
double ESP32PWM::writeNote(note_t note, uint8_t octave) {
	resolutionBits=10;
	return ledcWriteNote(getChannel(), note, octave);
}
uint32_t ESP32PWM::read() {
	return ledcRead(getChannel());
}
double ESP32PWM::readFreq() {
	return ledcReadFreq(getChannel());
}
void ESP32PWM::attachPin(uint8_t pin) {
	attach(pin);
	ledcAttachPin(pin, getChannel());
}
void ESP32PWM::attachPin(uint8_t pin, double freq, uint8_t resolution_bits) {
	setup(freq, resolution_bits);
	attachPin(pin);
}

void ESP32PWM::detachPin(uint8_t pin) {
	detach();
	return ledcDetachPin(pin);
}

ESP32PWM* pwmFactory(int pin) {
	for (int i = 0; i < NUM_PWM; i++)
		if (ChannelUsed[i] != NULL) {
			if (ChannelUsed[i]->getPin() == pin)
				return ChannelUsed[i];
		}
	return NULL;
}