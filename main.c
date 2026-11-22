//red led = PD5 (OCR0B) green led = PD6 (OCR0A) button = PD2 
//check atmega328p datasheet for register descriptions 



/*===========================================================================================================================================*
 |      +                                                                                                                              *     |
 |                                                                  *                                                                        |
 |*                                         *                   CONSTANTS                    *                                              *| 
 |                                                                  *                                                                        |
 |      +                                                                                                                              *     |
 *===========================================================================================================================================*/


//timer0 constants for PWM 
#define TCCR0A 	(*(volatile unsigned char *)	0x44)  
#define TCCR0B 	(*(volatile unsigned char *)	0x45)  
#define OCR0A	(*(volatile unsigned char *)	0x47)
#define OCR0B	(*(volatile unsigned char *)	0x48)


#define SREG 	(*(volatile unsigned char *)	0x5f)  

#define PCICR 	(*(volatile unsigned char *)	0x68)  
#define PCMSK2 	(*(volatile unsigned char *)	0x6D)  

//timer1 constants to use as hardware timer
#define TCCR1A	(*(volatile unsigned char *)	0x80)
#define TCCR1B	(*(volatile unsigned char *)	0x81)
#define TCNT1L	(*(volatile unsigned char *)	0x84)
#define TCNT1H	(*(volatile unsigned char *)	0x85)
#define OCR1AL	(*(volatile unsigned char *)	0x88)
#define OCR1AH	(*(volatile unsigned char *)	0x89)
#define TIMSK1	(*(volatile unsigned char *)	0x6F)

//timer2 for same
#define TCCR2A 	(*(volatile unsigned char *)	0xB0)  
#define TCCR2B 	(*(volatile unsigned char *)	0xB1)  
#define OCR2A	(*(volatile unsigned char *)	0xB3)

#define PORTB 	(*(volatile unsigned char *)	0x25)  
#define DDRB	(*(volatile unsigned char *)	0x24)
#define PINB	(*(volatile unsigned char *)	0x23)

#define PORTD 	(*(volatile unsigned char *)	0x2B)  
#define DDRD	(*(volatile unsigned char *)	0x2A)
#define PIND	(*(volatile unsigned char *)	0x29)

#define BIT0	((unsigned char)		1)
#define BIT1	((unsigned char)		1 << 1)
#define BIT2	((unsigned char)		1 << 2)
#define BIT3	((unsigned char)		1 << 3)
#define BIT4	((unsigned char)		1 << 4)
#define BIT5	((unsigned char)		1 << 5)
#define BIT6	((unsigned char)		1 << 6)
#define BIT7	((unsigned char)		1 << 7)

#define GREEN 	OCR0A
#define RED 	OCR0B
#define BUTTON 	BIT2

//variables
volatile unsigned char juddered = 0;
volatile unsigned char pressed = 0;
volatile unsigned char taken = 0; //0 = red lit, 1 = green lit

unsigned char intensity = 64;

void stall_for_judder(void) {
	for(volatile unsigned long i = 0; i < 40000UL; i++) {

	}
}
void start_judder(void) {
	//PORTB &= ~BIT5;
	juddered = 1;	
	TIMSK1 |= BIT1; 
	TCCR1A |= BIT7;
	OCR1AH = 0x13;
	OCR1AL = 0x87;
	TCCR1B = (BIT0 | BIT1 | BIT3);
}

void blink_led(void) {
	PORTB &= ~BIT5;
	stall_for_judder();
	PORTB |= BIT5;
}

//interrupt fires when the timer reaches its number
__attribute__ ((signal, used, externally_visible))
void __vector_11(void) {
	SREG &= ~BIT7;
	TCCR1B = 0; 
	juddered = 0;
	//PORTB |= BIT5;
	TIMSK1 = 0;
	TCCR1A = 0;
	SREG |= BIT7;
	PCMSK2 |= BIT2;
}


__attribute__ ((signal, used, externally_visible))
void __vector_5(void) {
	//latch previous button state
	static unsigned char previous = 1;	
	unsigned char now =  (PIND >> 2) & 1;
	pressed = (previous & ~now) ? 1 : 0; 
	previous = now;

	PORTB &= ~BIT5;
	PCMSK2 &= ~BIT2;
	if(!pressed) {
		PORTB &= ~BIT5;
		SREG |= BIT7;
	//	blink_led();
		pressed = 0;
		start_judder();
		return;
	} 
	PORTB |= BIT5;
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
	//PCMSK2 |= BIT2;
	taken = !taken;
	start_judder();
}

int main(void) {
	//unsigned char pressed = 0;
	unsigned char prev_pressed = 0;	
	unsigned char state_has_changed = 0;

	SREG |= BIT7;

	PCICR |= BIT2;
	PCMSK2 |= BIT2;

	
	DDRB |= BIT5;
	PORTB |= BIT5;

	DDRD &= ~BIT2;	
	DDRD |= (BIT5);
	DDRD |= BIT6;
	PORTD |= BIT2;
	

	//set OC0B to fast PWM, non inverting mode, attach red to pwm 
	TCCR0A = (BIT0 | BIT1| BIT5);
	//set prescaler to clk/64 (976Hz at 16MHz)
	TCCR0B = (BIT0 | BIT1);

	//ensure green light is off
	PORTD &= ~BIT6;	

	RED = intensity;



	while(1) {
		if (0 == 1) {
			pressed = !(PIND & BUTTON);

			if(!pressed && prev_pressed) 
			{		
				start_judder();
				prev_pressed = 0;
				continue;
			}

			if(pressed && !prev_pressed) {
				state_has_changed = 1;
				prev_pressed = 1;
				start_judder();
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
}
