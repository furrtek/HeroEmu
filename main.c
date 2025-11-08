// Firmware for HEROEMU_A - W.E.B. power band emulator
// Reproduces the gauntlet IR signals for each hero
// https://furrtek.org/?a=wslinger
// For ATTiny85 (25 and 45 OK, just change device)
// Fuses: LHE 0xE2 0xDF 0xFF

// All IR codes tested OK in French ride

// Pinout
// 		RESET	VCC
//  IR  PB3		PB2	LED data
// Btn	PB4		PB1
// 		GND		PB0	LED power

// Comes out of deep sleep when button pressed
// Sleep current should be ~40uA
// Press button to change hero, timeout is 10 code cycles (more than enough to be detected by ride)

// Bit duration: 32.315ms
// Total duration: 903ms (28 bits)

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

typedef struct {
	uint8_t color[3];	// R G B
	uint8_t ir_data[4];
} t_hero;

// Color values adjusted for 3V operation (near blue Vf)
// ir_data is what IDs the different toys, all toys of the same type transmit the same code
t_hero hero[6] = {
	{ { 10, 20, 50 },	{ 0xE3, 0x04, 0x21, 0x00 }},	// Naked		White
	{ { 0, 20, 0 },		{ 0xE3, 0x00, 0xA0, 0x00 }},	// Base			Green
	{ { 0, 0, 80 },		{ 0xE3, 0x00, 0x01, 0x00 }},	// Spiderman	Blue
	{ { 10, 0, 0 },		{ 0xE3, 0x04, 0x00, 0x00 }},	// Ironman		Red
	{ { 10, 20, 0 },	{ 0xE3, 0x54, 0x00, 0x00 }},	// DrStrange	Orange
	{ { 10, 0, 50 },	{ 0xE3, 0x50, 0x20, 0x00 }}		// Wakanda		Purple
};

// From ws2812b.s
extern void output_grb(uint8_t * ptr, uint16_t count, uint8_t outbit);

volatile uint8_t tick = 0;
uint8_t buf[3] = { 0 };

ISR(TIMER0_COMPA_vect) {
	tick += 1;
}

void set_color(uint8_t * color) {
	buf[1] = color[0];
	buf[0] = color[1];
	buf[2] = color[2];
	output_grb(buf, sizeof(buf), 1 << 2);	// LED on PB2
}

ISR (PCINT0_vect) {
	MCUCR &= ~(1 << SE);	// Disable sleep mode
}

void enter_sleep() {
	PORTB = 0b00010000;		// Cut power to LEDs, keep button pull-up active
	MCUCR |= (1 << SM1);
	MCUCR |= (1 << SE);
	__asm__ __volatile__("sleep" "\n\t"::);
}

uint8_t prev_btn = 0, btn = 0, dips = 0, idx = 0;
uint8_t timeout = 0;

void init() {
	_delay_ms(100);

	PORTB = 0b00010101;
	DDRB = 0b00001101;
	
	ADCSRA &= ~(1<<ADEN);	// Disable ADC

	// Refresh rate
	TCCR0A = 0b00000010;
	TCCR0B = 0b00000101;	// 8M/1024
	OCR0A = 127;			// ~60Hz
	TIMSK = 0b00010000;		// OCIE0A

	_delay_ms(100);

	prev_btn = 0;
	btn = 0;
	dips = 1;
	idx = 0;
	timeout = 0;
	set_color(hero[dips].color);

	// Enable PCIE for wake-up
	GIMSK |= (1 << PCIE);
	PCMSK |= (1 << PCINT4);
}

int main() {
	wdt_disable();
	init();
	sei();

	for (;;) {
		// ~30Hz update
		if (tick >= 2) {
			tick = 0;

			// IR LED control
			if ((hero[dips].ir_data[idx >> 3] << (idx & 7)) & 0x80)
				PORTB |= (1<<3);
			else
				PORTB &= ~(1<<3);

			// IR code looping
			if (idx >= 27) {
				idx = 0;
				if (timeout >= 10) {
					// 10 repeats * 0.9s =~ 9s
					enter_sleep();
					init();
				} else
					timeout++;
			} else
				idx++;
				
			// Button falling edge detect
			btn = PINB & 0b00010000;
			if (prev_btn && !btn) {
				timeout = 0;
				if (dips >= 5)
					dips = 0;
				else
					dips++;
				set_color(hero[dips].color);
				idx = 0;
			}
			prev_btn = btn;
		}
	}

	return 0;
}


