#define TCCR0A 	(*(volatile unsigned char *)0x44)  
#define TCCR0B 	(*(volatile unsigned char *)0x45)  
#define OCR0A	(*(volatile unsigned char *)0x47)
#define OCR0B	(*(volatile unsigned char *)0x48)

#define TCCR2A 	(*(volatile unsigned char *)0xB0)  
#define TCCR2B 	(*(volatile unsigned char *)0xB1)  
#define OCR2A	(*(volatile unsigned char *)0xB3)


#define PORTB 	(*(volatile unsigned char *)0x25)  
#define DDRB	(*(volatile unsigned char *)0x24)
#define PINB	(*(volatile unsigned char *)0x23)

#define PORTD 	(*(volatile unsigned char *)0x2B)  
#define DDRD	(*(volatile unsigned char *)0x2A)
#define PIND	(*(volatile unsigned char *)0x29)

#define BIT0	((unsigned char)1)
#define BIT1	((unsigned char)1 << 1)
#define BIT2	((unsigned char)1 << 2)
#define BIT3	((unsigned char)1 << 3)
#define BIT4	((unsigned char)1 << 4)
#define BIT5	((unsigned char)1 << 5)
#define BIT6	((unsigned char)1 << 6)
#define BIT7	((unsigned char)1 << 7)

#define GREEN 	OCR2A
#define RED 	OCR0B
#define BUTTON 	BIT2


//ensure you don't read judder flips
void stall_for_judder(void) {
	for(volatile unsigned long i = 0; i < 40000UL; i++) {
		
	}
}

int main(void) {
	unsigned char intensity = 64;
	unsigned char pressed = 0;
	unsigned char prev_pressed = 0;	
	unsigned char taken = 0;
	unsigned char state_has_changed = 0;

	DDRD &= ~BIT2;	
	DDRD |= (BIT5 | BIT6);
	PORTD |= BIT2;
	
	DDRB |= BIT3;

	TCCR0A = (BIT0 | BIT1| BIT5);
	TCCR0B = (BIT0 | BIT1);

	TCCR2A = (BIT0 | BIT1);
	TCCR2B = BIT2;

	PORTB &= ~BIT3;	

	RED = intensity;



	while(1) {
		pressed = !(PIND & BUTTON);

		if(!pressed && prev_pressed) 
		{		
			stall_for_judder();
			prev_pressed = 0;
		}

		if(pressed && !prev_pressed) {
			state_has_changed = 1;
			stall_for_judder();
			prev_pressed = 1;
		}

		if(state_has_changed) {
			if(taken) {
				TCCR2A &= ~BIT7;
				PORTB &= ~BIT3;
				TCCR0A |= BIT5;
				RED = intensity;
			}
			if(!taken) {
				TCCR0A &= ~BIT5;
				PORTD &= ~BIT5;
				TCCR2A |= BIT7;
				GREEN = intensity;
			}
			taken = !taken;
			state_has_changed = 0;
		}
	}

}
