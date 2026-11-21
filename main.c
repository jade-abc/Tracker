//red led = PD5 (OC0A) green led = PD6 (OC0B) button = PD2 
//check atmega328p datasheet for register descriptions 


//timer0 constants for PWM 
#define TCCR0A 	(*(volatile unsigned char *)0x44)  
#define TCCR0B 	(*(volatile unsigned char *)0x45)  
#define OCR0A	(*(volatile unsigned char *)0x47)
#define OCR0B	(*(volatile unsigned char *)0x48)

//timer2 for same
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

#define GREEN 	OCR0A
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
	unsigned char taken = 0; //0 = red lit, 1 = green lit
	unsigned char state_has_changed = 0;

	DDRD &= ~BIT2;	
	DDRD |= (BIT5);
	DDRD |= BIT6;
	PORTD |= BIT2;
	

	//set OC0B to fast PWM, non inverting mode 
	TCCR0A = (BIT0 | BIT1| BIT5);
	//set prescaler to clk/64 (976Hz at 16MHz)
	TCCR0B = (BIT0 | BIT1);

	//ensure green light is off
	PORTD &= ~BIT6;	

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
			//changing to PWM mode it became clear that fast PWM still emits a spike at 0 giving a faint glow, the easiest fix I 
			//could think of was detaching the pins from PWM completely and then turning them off as GPIO pins 
			if(taken) {
				//detach the green pin from timer2 and drive it to a LOW signal
				TCCR0A &= ~BIT7;
				PORTD &= ~BIT6;
				//attach the red pin to timer0 and set it to be intensity
				TCCR0A |= BIT5;
				RED = intensity;
			}
			if(!taken) {
				//detach the red pin from timer0 and drive it to a LOW signal
				TCCR0A &= ~BIT5;
				PORTD &= ~BIT5;
				//attach the green pin to timer2 and set it to be intensity
				TCCR0A |= BIT7;
				GREEN = intensity;
			}
			taken = !taken;
			state_has_changed = 0;
		}
	}
}
