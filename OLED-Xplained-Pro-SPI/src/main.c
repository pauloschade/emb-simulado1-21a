#include <asf.h>
#include "helpers.h"

#include "oled/gfx_mono_ug_2832hsweg04.h"
#include "oled/gfx_mono_text.h"
#include "oled/sysfont.h"

#define LED_1_PIO PIOA
#define LED_1_PIO_ID ID_PIOA
#define LED_1_IDX 0
#define LED_1_IDX_MASK (1 << LED_1_IDX)

#define LED_2_PIO PIOC
#define LED_2_PIO_ID ID_PIOC
#define LED_2_IDX 30
#define LED_2_IDX_MASK (1 << LED_2_IDX)

#define LED_3_PIO PIOB
#define LED_3_PIO_ID ID_PIOB
#define LED_3_IDX 2
#define LED_3_IDX_MASK (1 << LED_3_IDX)

#define BUT1_PIO PIOD
#define BUT1_PIO_ID	ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_IDX_MASK (1 << BUT1_PIO_IDX)

#define BUT2_PIO PIOC
#define BUT2_PIO_ID	ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_IDX_MASK (1 << BUT2_PIO_IDX)

#define BUT3_PIO PIOA
#define BUT3_PIO_ID	ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_IDX_MASK (1 << BUT3_PIO_IDX)

enum states {
	set = 1,
	lock = 2,
	open = 3,
	block = 4
};

volatile char but1_flag;
volatile char but2_flag;
volatile char but3_flag;
volatile char alarm_rtt = 0;

void all_flags_zero() {
	but1_flag = 0;
	but2_flag = 0;
	but3_flag = 0;
}


void erase_oled() {
	gfx_mono_draw_string("             ", 0, 5, &sysfont);
	gfx_mono_draw_string("             ", 0, 16, &sysfont);
}

void draw_oled(char str[10]) {
	gfx_mono_draw_string(str, 0, 16, &sysfont);
}

void pin_toggle(Pio *pio, uint32_t mask) {
	pio_get_output_data_status(pio, mask) ? pio_clear(pio, mask) : pio_set(pio,mask);
}

void toggle_all() {
	pin_toggle(LED_1_PIO, LED_1_IDX_MASK);
	pin_toggle(LED_2_PIO, LED_2_IDX_MASK);
	pin_toggle(LED_3_PIO, LED_3_IDX_MASK);
}

void all_leds_on() {
	pio_clear(LED_1_PIO, LED_1_IDX_MASK);
	pio_clear(LED_2_PIO, LED_2_IDX_MASK);
	pio_clear(LED_3_PIO, LED_3_IDX_MASK);
}

void but1_callback(void) {
	but1_flag = 1;
}

void but2_callback(void) {
	but2_flag = 1;
}

void but3_callback(void) {
	but3_flag = 1;
}

int handle_passwd(int passwd[], int *p_n) {
	int n = *p_n;
	if(but1_flag) {
		passwd[n] = 1;
		n++;
		but1_flag = 0;
	}
	if(but2_flag) {
		passwd[n] = 2;
		n++;
		but2_flag = 0;
	}
	if(but3_flag) {
		passwd[n] = 3;
		n++;
		but3_flag = 0;
	}
	
	if (n != *p_n) {
		*p_n = n;
		return 1;
	}
	
	return 0;
}

int check_passwd(int passwd[], int input[], int n) {
	for (int i = 0; i < n; i++) {
		int a = passwd[i];
		int b = input[i];
		if(passwd[i] != input[i]) {
			return 0;
		}
	}
	return 1;
}

/************************************************************************/
/* handlers                                                              */
/************************************************************************/

void RTT_Handler(void) {
	uint32_t ul_status;

	ul_status = rtt_get_status(RTT);
	
	//alarm
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		alarm_rtt = 1;
	}
	
	//time
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		//TODO
	}

}

void TC0_Handler(void) {
	
	volatile uint32_t status = tc_get_status(TC0, 0);
	toggle_all();
}

void TC1_Handler(void) {
	
	volatile uint32_t status = tc_get_status(TC0, 1);
	toggle_all();
}

/************************************************************************/
/* init                                                              */
/************************************************************************/

void io_init(void) {
  pmc_enable_periph_clk(LED_1_PIO_ID);
  pmc_enable_periph_clk(LED_2_PIO_ID);
  pmc_enable_periph_clk(LED_3_PIO_ID);
  pmc_enable_periph_clk(BUT1_PIO_ID);
  pmc_enable_periph_clk(BUT2_PIO_ID);
  pmc_enable_periph_clk(BUT3_PIO_ID);

  pio_configure(LED_1_PIO, PIO_OUTPUT_1, LED_1_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_2_PIO, PIO_OUTPUT_1, LED_2_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_3_PIO, PIO_OUTPUT_1, LED_3_IDX_MASK, PIO_DEFAULT);

  pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);

  pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_IDX_MASK, PIO_IT_FALL_EDGE,
  but1_callback);
  pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_IDX_MASK, PIO_IT_FALL_EDGE,
  but2_callback);
  pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_IDX_MASK, PIO_IT_FALL_EDGE,
  but3_callback);

  pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
  pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
  pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);

  pio_get_interrupt_status(BUT1_PIO);
  pio_get_interrupt_status(BUT2_PIO);
  pio_get_interrupt_status(BUT3_PIO);

  NVIC_EnableIRQ(BUT1_PIO_ID);
  NVIC_SetPriority(BUT1_PIO_ID, 4);

  NVIC_EnableIRQ(BUT2_PIO_ID);
  NVIC_SetPriority(BUT2_PIO_ID, 4);

  NVIC_EnableIRQ(BUT3_PIO_ID);
  NVIC_SetPriority(BUT3_PIO_ID, 4);
}

int main(void) {
	
  board_init();
  sysclk_init();
  delay_init();
  io_init();
  gfx_mono_ssd1306_init();
	
	TC_init(TC0, ID_TC0, 0, 5);
	TC_init(TC0, ID_TC1, 1, 2);
	
	int total_length = 6;
	int passwd[total_length];
	int input_passwd[total_length];
	int n = 0;
	
	int update_display = 0;
	int blocked = 0;
	
	
	enum states cur_state = set;
	draw_oled("set passwd");
  
  while (1) {
    switch(cur_state) {
			case(set):
				if (n > 5) {
					n = 0;
					erase_oled();
					all_leds_on();
					cur_state = lock;
					draw_oled("locked");
					} else {
					update_display = handle_passwd(passwd, &n);
					if (update_display) {
						if (n == 1) {
							erase_oled();
						}
						gfx_mono_draw_string("*", n * 10, 16, &sysfont);
					}

				}
				break;
			case(lock):
				if(alarm_rtt) {
					n = 0;
					erase_oled();
					draw_oled("lock");
					alarm_rtt = 0;
				}	
				if (n > 5) {
					n = 0;
					erase_oled();
					int check = check_passwd(passwd, input_passwd, total_length);
					if(check) {
						cur_state = open;
						draw_oled("open");
						toggle_all();
						tc_start(TC0,0);
					} else {
						cur_state = block;
						if(blocked) {
							RTT_init(4, 32, RTT_MR_ALMIEN);
							blocked = 0;
						} else {
								RTT_init(4, 16, RTT_MR_ALMIEN);
								blocked = 1;
						}
						tc_start(TC0, 1);
						draw_oled("block");
					}
				} else {
					update_display = handle_passwd(input_passwd, &n);
					if (update_display) {
						RTT_init(4, 16, RTT_MR_ALMIEN);
						if (n == 1) {
							erase_oled();
						}
						gfx_mono_draw_string("*", n * 10, 16, &sysfont);
					}
				}
				break;
			case(open):
				if(but1_flag) {
					erase_oled();
					draw_oled("lock");
					cur_state = lock;
					tc_stop(TC0, 0);
					all_flags_zero();
				}
				break;
			case(block):
				if(alarm_rtt) {
					erase_oled();
					draw_oled("lock");
					cur_state = lock;
					alarm_rtt = 0;
					tc_stop(TC0, 1);
					all_flags_zero();
				}
				break;
		}
  }
}
