#define PORTB 	(*(volatile unsigned char *)0x25)  
#define DDRB	(*(volatile unsigned char *)0x24)
#define PINB	(*(volatile unsigned char *)0x23)

#define PORTD 	(*(volatile unsigned char *)0x2B)  
#define DDRD	(*(volatile unsigned char *)0x2A)
#define PIND	(*(volatile unsigned char *)0x29)

#define BIT0	((unsigned char)1)
#define BIT1	((unsigned char)1 << 1)
#define BIT2	((unsigned char)1 << 2)


#define GREEN BIT0
#define RED BIT1

void stall_judder(void) {
	for(volatile unsigned long i = 0; i < 40000UL; i++) {
		
	}
}

int main(void) {
	unsigned char pressed = 0;
	unsigned char prev_pressed = 0;	
	unsigned char taken = 0;
	unsigned char state_has_changed = 0;

	DDRB |= GREEN;
	DDRB |= RED;
	PORTB |= RED;

	DDRD &= ~BIT2;	
	PORTD |= BIT2;

	while(1) {
		pressed = !(PIND & BIT2);

		if(!pressed && prev_pressed) prev_pressed = 0;

		if(pressed && !prev_pressed) {
			state_has_changed = 1;
			stall_judder();
			prev_pressed = 1;
		}

		if(state_has_changed) {
			unsigned char change = (PINB & GREEN) ? 1 : 0;
			if(change) {
				PORTB &= ~GREEN;
				PORTB |= RED;
			}
			if(!change) {
				PORTB &= ~RED;
				PORTB |= GREEN;
			}
			state_has_changed = false;
		}
	}

}
