//    Arduino PPM Generator
//    Copyright (C) 2015-2020  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <SimpleModbusSlave.h>

// Maximum number of channels
#define MAX_COUNT 16     

enum State {
	Pulse,             // N-th channel pulse
	StartSync,         // Start of sync pulse
	ContinueSync,      // Sync pulse continuation
	FinishSync         // Sync pulse end
};

// little endian
typedef union {
	unsigned long raw;
	struct {
		word low;
		word high;
	};
} long_t;

typedef union __attribute__ ((packed)) {
	word raw[6+MAX_COUNT];
	struct __attribute__ ((packed)) {
		word quant;               // 1 microsec per system frequency clock
		word max_count;           // Maximum number of channels
		word state;               // 2 - On (inversion) / 1 - On / 0 - Off
		word count;               // Number of channels (0 ... MAX_COUNT)
		word pause;               // Duration of pause (in cycles of the system frequency)
		long_t sync;              // The duration of the synchronization pulse (in cycles of the system frequency)
		word channel[MAX_COUNT];  // Pulse duration (in cycles of the system frequency)
	};
} regs_t;

regs_t tmp;                   // Temporary dataset
regs_t ppm;                   // Working dataset
volatile byte state = Pulse;  // Current state
volatile byte current = 0;    // Current channel number
SimpleModbusSlave slave(1);   // Modbus slave с адресом 1

byte const modbus_registers_count = sizeof(regs_t) / sizeof(word);

// Controller initialization
void setup() {
	// Peripheral initialization
	pinMode(9, OUTPUT);         // Debug
	pinMode(10, OUTPUT);        // PPM
	digitalWrite(10, HIGH);

	// Initializing Channel Values
	tmp.state        = 0;
	tmp.max_count    = MAX_COUNT;
	tmp.count        = 8;
	tmp.quant        = F_CPU / 1000000;
	tmp.pause        = F_CPU / 1000000 * 200;
	tmp.sync.raw     = F_CPU / 1000000 * 22500 - F_CPU / 1000000 * 300 * 8;

	// Channel duration 300 µs
	for (byte i = 0; i < MAX_COUNT; i++) tmp.channel[i] = 300 * (unsigned long) tmp.quant;

	// Настраиваем MODBUS
	slave.setup(115200);

#ifdef DEBUG
	Serial.println(tmp.state);
	Serial.println(tmp.count);
	Serial.println(tmp.pause);
	Serial.println(tmp.sync.low);
	Serial.println(tmp.sync.high);
	Serial.println(tmp.sync.raw);
	
	for (byte i = 0; i < MAX_COUNT; i++) {
		Serial.println(tmp.channel[i]);
	}
#endif
}

// Main loop
void loop() {
	word lastState = tmp.state;
	slave.loop(tmp.raw, sizeof(tmp) / sizeof(tmp.raw[0]));
	if (lastState != tmp.state) {
		tmp.state > 0 ? Start() : Stop();
	}
}

// Run generation
void Start() {
	cli();                                           // Disable global interrupt
	ppm    = tmp;                                    // Copy dataset
	TIMSK1 = B00000001;                              // Enable Timer Interrupt
	TCCR1A = ppm.state == 2 ? B00110011 : B00100011; // FAST PWM MODE 15
	TCCR1B = B00011001;                              // Prescaler = 1
	TCCR1C = B00000000;                              //
	OCR1A  = ppm.channel[0];                         // Duration of the first impulse with pause
	OCR1B  = ppm.channel[0] - ppm.pause;             // The duration of the first pulse without a pause
	current = 1;                                     //
	sei();                                           // Reenable global interrupt
}

// Stop generation
void Stop() {
	cli();             // Disable global interrupt
	TCCR1B = 0;        // Stop the timer
	TIMSK1 = 0;        // Disable timer interrupt
	sei();             // Reenable global interrupt
}

// Timer Overflow interrupt
ISR(TIMER1_OVF_vect) {

	switch (state) {
		
	// Импульс n-го канала
	case Pulse:		
		OCR1A = ppm.channel[current];                 // The duration of the current pulse with pause
		OCR1B = ppm.channel[current] - ppm.pause;     // The duration of the current pulse without a pause  

		// We proceed to the formation of a clock pulse
		if (++current == ppm.count) state = (ppm.sync.high == 0) ? FinishSync : ppm.sync.low > ppm.pause ? ContinueSync : StartSync;                               
		break;

	// If the minor ppm.sync.low is less than ppm.pause, then we start the impulse with the duration ppm.pause and
	// on the next pass ppm.sync.low will be more than ppm.pause
	case StartSync:
		OCR1B = OCR1A = ppm.pause;
		ppm.sync.raw -= ppm.pause;
		state = ppm.sync.high ? ContinueSync : FinishSync;
		break;

	// If ppm.sync.high > 0 start the pulse of maximum duration
	case ContinueSync: 
		OCR1A = OCR1B = 0xFFFF;       
		if (--ppm.sync.high == 0) state = FinishSync;
		break;

	// Completion of sync pulse
	case FinishSync: 
		OCR1A   = ppm.sync.low;
		OCR1B   = ppm.sync.low - ppm.pause;
		ppm     = tmp;      // Update setting at the end of the transfer.  // TODO: Add block from partial update.
		state   = Pulse;
		current = 0;
	}
}
