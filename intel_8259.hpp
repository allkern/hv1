#pragma once

#include <cstdint>

enum signal_trigger_mode_t : int {
	ST_NONE			= 0b0000,
	ST_RISING_EDGE  = 0b0001,
	ST_FALLING_EDGE = 0b0010,
	ST_LEVEL_HIGH   = 0b0100,
	ST_LEVEL_LOW    = 0b1000
};

typedef void (*signal_trigger_callback_t)(bool);

struct signal_t {
	int mode = ST_NONE;
	signal_trigger_callback_t callback = nullptr;

	bool prev = false, value = false, triggered = false;
	bool* variable = nullptr;

	void check_triggers() {
		if (mode & ST_LEVEL_HIGH  ) { triggered =  prev &&  value; if (triggered && callback) callback(value); }
		if (mode & ST_FALLING_EDGE) { triggered =  prev && !value; if (triggered && callback) callback(value); }
		if (mode & ST_RISING_EDGE ) { triggered = !prev &&  value; if (triggered && callback) callback(value); }
		if (mode & ST_LEVEL_LOW   ) { triggered = !prev && !value; if (triggered && callback) callback(value); }
	}

	void write(bool new_value) {
		prev = value;
		value = new_value;
	
		check_triggers();
	}

	void update() {
		prev = value;
		value = *variable;

		check_triggers();
	}

	bool read() {
		return value;
	}
};

struct dev_pic_intel_8259_t {
	uint8_t r[3] = { 0 };

	signal_t signal;

	signal_t irq[8],
	  	 	 intr, inta,
	  	 	 d[8],
	  	 	 cas[3];
} pic;