#include <Arduino.h>
#include "HardwareSerial.h"

int tbl_sinus_scale = 255;

typedef struct _int_bresenham {
	int ante, conseq;
	int ante_err, conseq_err;
} int_bresenham;

/* 1.2 microsecs */
static inline byte dither_bresenham(int_bresenham *br) {
	byte ret = 0;
	if (br->ante_err < br->ante && br->conseq_err < br->conseq) {
		br->conseq_err += br->ante;
		br->ante_err += br->conseq;
	}
	if (br->ante <= br->ante_err) {
		++ret;
		br->ante_err -= br->ante;
	}
	if (br->conseq <= br->conseq_err) {
		ret |= 2;
		br->conseq_err -= br->conseq;
	}
	return ret;
}

int led = 9;

// the setup routine runs once when you press reset:
void setup_timer1 () {
	noInterrupts();
	// disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;

	OCR1A = 1;            // compare match register 16MHz/256/2Hz
	TCCR1B |= (1 << WGM12);   // CTC mode
	TCCR1B |= (1 << CS10)|(1 << CS11);    // 64 prescaler
	TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
	interrupts();
}
void setup() {
	// initialize the digital pin as an output.
	pinMode(led, OUTPUT);
	setup_timer1();
//	Serial.begin(115200);
}

byte current_state = 0;
int_bresenham current_br = { 0, 0, 0, 0 };

/* 1.25 microsecs */
static inline void ledWriteB(byte pin) {
	pin -= 8;
	byte pins = PORTB;
	byte old_pins = pins;
	if (current_state & 1) {
		pins |= (HIGH << pin);
		--current_state;
	} else if (current_state & 2) {
		pins &= ~(HIGH << pin);
		current_state -= 2;
	}
	if (old_pins != pins)
		PORTB = pins;
}

void loop() {
	short pot_value = analogRead(A0);
//	analogWrite(led, pot_value / 4);
	current_br.ante = 1023 - pot_value;
	current_br.conseq = pot_value;
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
	if (current_state == 0) {
		current_state = dither_bresenham(&current_br);
	}
	ledWriteB(led);
}


int main(void) {
	init();
	setup();
	//endless loop
	for (;;) {
		loop();
	}
	return 0;
}

