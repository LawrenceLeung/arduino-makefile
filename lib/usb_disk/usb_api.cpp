/* USB API for Teensy USB Development Board
 * http://www.pjrc.com/teensy/teensyduino.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "usb_common.h"
#include "usb_private.h"
#include "usb_api.h"
#include "wiring.h"


#define CTRL  64
#define SHIFT 128

static const uint16_t PROGMEM keycodes[] = {
	0,			// 0
	KEY_A + CTRL,		// 1
	KEY_B + CTRL,		// 2
	KEY_C + CTRL,		// 3
	KEY_D + CTRL,		// 4
	KEY_E + CTRL,		// 5
	KEY_F + CTRL,		// 6
	KEY_G + CTRL,		// 7
	KEY_H + CTRL,		// 8
	KEY_TAB,		// 9
	KEY_J + CTRL,		// 10
	KEY_K + CTRL,		// 11
	KEY_L + CTRL,		// 12
	KEY_ENTER,		// 13
	KEY_N + CTRL,		// 14
	KEY_O + CTRL,		// 15
	KEY_P + CTRL,		// 16
	KEY_Q + CTRL,		// 17
	KEY_R + CTRL,		// 18
	KEY_S + CTRL,		// 19
	KEY_T + CTRL,		// 20
	KEY_U + CTRL,		// 21
	KEY_V + CTRL,		// 22
	KEY_W + CTRL,		// 23
	KEY_X + CTRL,		// 24
	KEY_Y + CTRL,		// 25
	KEY_Z + CTRL,		// 26
	KEY_ESC,		// 27
	KEY_BACKSLASH + CTRL,	// 28
	KEY_RIGHT_BRACE + CTRL,	// 29
	KEY_6 + CTRL + SHIFT,	// 30
	KEY_MINUS + CTRL + SHIFT, // 31
	KEY_SPACE,		// 32  
	KEY_1 + SHIFT,		// 33 !
	KEY_QUOTE + SHIFT,	// 34 "
	KEY_3 + SHIFT,		// 35 #
	KEY_4 + SHIFT,		// 36 $
	KEY_5 + SHIFT,		// 37 %
	KEY_7 + SHIFT,		// 38 &
	KEY_QUOTE,		// 39 '  
	KEY_9 + SHIFT,		// 40 ( 
	KEY_0 + SHIFT,		// 41 )
	KEY_8 + SHIFT,		// 42 *
	KEY_EQUAL + SHIFT,	// 43 +
	KEY_COMMA,		// 44 ,
	KEY_MINUS,		// 45 -
	KEY_PERIOD,		// 46 .
	KEY_SLASH,		// 47 /
	KEY_0,			// 48 0
	KEY_1,			// 49 1
	KEY_2,			// 50 2
	KEY_3,			// 51 3
	KEY_4,			// 52 4
	KEY_5,			// 53 5
	KEY_6,			// 54 6
	KEY_7,			// 55 7
	KEY_8,			// 55 8
	KEY_9,			// 57 9
	KEY_SEMICOLON + SHIFT,	// 58 :
	KEY_SEMICOLON,		// 59 ;
	KEY_COMMA + SHIFT,	// 60 <
	KEY_EQUAL,		// 61 =
	KEY_PERIOD + SHIFT,	// 62 >
	KEY_SLASH + SHIFT,	// 63 ?
	KEY_2 + SHIFT,		// 64 @
	KEY_A + SHIFT,		// 65 A
	KEY_B + SHIFT,		// 66 A
	KEY_C + SHIFT,		// 67 A
	KEY_D + SHIFT,		// 68 A
	KEY_E + SHIFT,		// 69 A
	KEY_F + SHIFT,		// 70 A
	KEY_G + SHIFT,		// 71 A
	KEY_H + SHIFT,		// 72 A
	KEY_I + SHIFT,		// 73 A
	KEY_J + SHIFT,		// 74 A
	KEY_K + SHIFT,		// 75 A
	KEY_L + SHIFT,		// 76 A
	KEY_M + SHIFT,		// 77 A
	KEY_N + SHIFT,		// 78 A
	KEY_O + SHIFT,		// 79 A
	KEY_P + SHIFT,		// 80 A
	KEY_Q + SHIFT,		// 81 A
	KEY_R + SHIFT,		// 82 A
	KEY_S + SHIFT,		// 83 A
	KEY_T + SHIFT,		// 84 A
	KEY_U + SHIFT,		// 85 A
	KEY_V + SHIFT,		// 86 A
	KEY_W + SHIFT,		// 87 A
	KEY_X + SHIFT,		// 88 A
	KEY_Y + SHIFT,		// 89 A
	KEY_Z + SHIFT,		// 90 A
	KEY_LEFT_BRACE,		// 91 [
	KEY_BACKSLASH,		// 92 
	KEY_RIGHT_BRACE,	// 93 ]
	KEY_6 + SHIFT,		// 94 ^
	KEY_MINUS + SHIFT,	// 95 _
	KEY_TILDE,		// 96 `
	KEY_A,			// 97
	KEY_B,			// 98
	KEY_C,			// 99
	KEY_D,			// 100
	KEY_E,			// 101
	KEY_F,			// 102
	KEY_G,			// 103
	KEY_H,			// 104
	KEY_I,			// 105
	KEY_J,			// 106
	KEY_K,			// 107
	KEY_L,			// 108
	KEY_M,			// 109
	KEY_N,			// 110
	KEY_O,			// 111
	KEY_P,			// 112
	KEY_Q,			// 113
	KEY_R,			// 114
	KEY_S,			// 115
	KEY_T,			// 116
	KEY_U,			// 117
	KEY_V,			// 118
	KEY_W,			// 119
	KEY_X,			// 120
	KEY_Y,			// 121
	KEY_Z,			// 122
	KEY_LEFT_BRACE + SHIFT,	// 123 {
	KEY_BACKSLASH + SHIFT,	// 124 |
	KEY_RIGHT_BRACE + SHIFT,// 125 }
	KEY_TILDE + SHIFT,	// 126 ~
	KEY_BACKSPACE		// 127
};



void usb_keyboard_class::write(uint8_t c)
{
	uint8_t key, modifier=0;

	if (c >= 128) return;
	key = pgm_read_byte(keycodes + c);
	if (key & SHIFT) modifier |= MODIFIERKEY_SHIFT;
	if (key & CTRL) modifier |= MODIFIERKEY_CTRL;
	keyboard_modifier_keys = modifier;
	keyboard_keys[0] = key & 0x3F;
	keyboard_keys[1] = 0;
	keyboard_keys[2] = 0;
	keyboard_keys[3] = 0;
	keyboard_keys[4] = 0;
	keyboard_keys[5] = 0;
	send_now();
	keyboard_modifier_keys = 0;
	keyboard_keys[0] = 0;
	send_now();
}


void usb_keyboard_class::set_modifier(uint8_t c)
{
	keyboard_modifier_keys = c;
}
void usb_keyboard_class::set_key1(uint8_t c)
{
	keyboard_keys[0] = c;
}
void usb_keyboard_class::set_key2(uint8_t c)
{
	keyboard_keys[1] = c;
}
void usb_keyboard_class::set_key3(uint8_t c)
{
	keyboard_keys[2] = c;
}
void usb_keyboard_class::set_key4(uint8_t c)
{
	keyboard_keys[3] = c;
}
void usb_keyboard_class::set_key5(uint8_t c)
{
	keyboard_keys[4] = c;
}
void usb_keyboard_class::set_key6(uint8_t c)
{
	keyboard_keys[5] = c;
}


void usb_keyboard_class::send_now(void)
{
        uint8_t i, intr_state, timeout;

        if (!usb_configuration) return;
        intr_state = SREG;
        cli();
        UENUM = KEYBOARD_ENDPOINT;
        timeout = UDFNUML + 50;
        while (1) {
                // are we ready to transmit?
                if (UEINTX & (1<<RWAL)) break;
                SREG = intr_state;
                // has the USB gone offline?
                if (!usb_configuration) return;
                // have we waited too long?
                if (UDFNUML == timeout) return;
                // get ready to try checking again
                intr_state = SREG;
                cli();
                UENUM = KEYBOARD_ENDPOINT;
        }
        UEDATX = keyboard_modifier_keys;
        UEDATX = 0;
        UEDATX = keyboard_keys[0];
        UEDATX = keyboard_keys[1];
        UEDATX = keyboard_keys[2];
        UEDATX = keyboard_keys[3];
        UEDATX = keyboard_keys[4];
        UEDATX = keyboard_keys[5];
        UEINTX = 0x3A;
        keyboard_idle_count = 0;
        SREG = intr_state;
}



void usb_serial_class::begin(long speed)
{
	// make sure USB is initialized
	usb_init();
	uint16_t begin_wait = (uint16_t)millis();
	while (1) {
		// wait for the host to finish enumeration
		if (usb_configuration) {
			delay(250);  // a little time for host to load a driver
			return;
		}
		// or for suspend mode (powered without USB)
		if (usb_suspended) {
			uint16_t begin_suspend = (uint16_t)millis();
			while (usb_suspended) {
				// must remain suspended for a while, because
				// normal USB enumeration causes brief suspend
				// states, typically under 0.1 second
				if ((uint16_t)millis() - begin_suspend > 250) {
					return;
				}
			}
		}
		// ... or a timout (powered by a USB power adaptor that
		// wiggles the data lines to keep a USB device charging)
		if ((uint16_t)millis() - begin_wait > 2500) return;
	}
}

void usb_serial_class::end(void)
{
	usb_shutdown();
	delay(25);
}


static volatile uint8_t prev_byte=0;

// number of bytes available in the receive buffer
uint8_t usb_serial_class::available(void)
{
        uint8_t c;

	c = prev_byte;  // assume 1 byte static volatile access is atomic
	if (c) return 1;
	c = readnext();
	if (c) {
		prev_byte = c;
		return 1;
	}
	return 0;
}

// get the next character, or -1 if nothing received
int usb_serial_class::read(void)
{
	uint8_t c;

	c = prev_byte;
	if (c) {
		prev_byte = 0;
		return c;
	}
	c = readnext();
	if (c) return c;
	return -1;
}

// get the next character, or 0 if nothing
uint8_t usb_serial_class::readnext(void)
{
        uint8_t c, c2, intr_state;

        // interrupts are disabled so these functions can be
        // used from the main program or interrupt context,
        // even both in the same program!
        intr_state = SREG;
        cli();
        if (!usb_configuration) {
                SREG = intr_state;
                return 0;
        }
        UENUM = DEBUG_RX_ENDPOINT;
try_again:
        if (!(UEINTX & (1<<RWAL))) {
                // no packet in buffer
                SREG = intr_state;
                return 0;
        }
        // take one byte out of the buffer
        c = UEDATX;
	if (c == 0) {
		// if we see a zero, discard it and
		// discard the rest of this packet
		UEINTX = 0x6B;
		goto try_again;
	}
        // if this drained the buffer, release it
        if (!(UEINTX & (1<<RWAL))) UEINTX = 0x6B;
        SREG = intr_state;
        return c;
}

// discard any buffered input
void usb_serial_class::flush()
{
        uint8_t intr_state;

        if (usb_configuration) {
                intr_state = SREG;
                cli();
                //UENUM = CDC_RX_ENDPOINT;
                while ((UEINTX & (1<<RWAL))) {
                        UEINTX = 0x6B;
                }
                SREG = intr_state;
        }
}

// transmit a character.
void usb_serial_class::write(uint8_t c)
{
        //static uint8_t previous_timeout=0;
        uint8_t timeout, intr_state;

        // if we're not online (enumerated and configured), error
        if (!usb_configuration) return;
        // interrupts are disabled so these functions can be
        // used from the main program or interrupt context,
        // even both in the same program!
        intr_state = SREG;
        cli();
        UENUM = DEBUG_TX_ENDPOINT;
        // if we gave up due to timeout before, don't wait again
#if 0
        if (previous_timeout) {
                if (!(UEINTX & (1<<RWAL))) {
                        SREG = intr_state;
                        return;
                }
                previous_timeout = 0;
        }
#endif
        // wait for the FIFO to be ready to accept data
        timeout = UDFNUML + TRANSMIT_TIMEOUT;
        while (1) {
                // are we ready to transmit?
                if (UEINTX & (1<<RWAL)) break;
                SREG = intr_state;
                // have we waited too long?  This happens if the user
                // is not running an application that is listening
                if (UDFNUML == timeout) {
                        //previous_timeout = 1;
                        return;
                }
                // has the USB gone offline?
                if (!usb_configuration) return;
                // get ready to try checking again
                intr_state = SREG;
                cli();
                UENUM = DEBUG_TX_ENDPOINT;
        }
        // actually write the byte into the FIFO
        UEDATX = c;
        // if this completed a packet, transmit it now!
        if (!(UEINTX & (1<<RWAL))) {
		UEINTX = 0x3A;
        	debug_flush_timer = 0;
	} else {
        	debug_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
	}
        SREG = intr_state;
}


// These are Teensy-specific extensions to the Serial object

// immediately transmit any buffered output.
// This doesn't actually transmit the data - that is impossible!
// USB devices only transmit when the host allows, so the best
// we can do is release the FIFO buffer for when the host wants it
void usb_serial_class::send_now(void)
{
        uint8_t intr_state;

        intr_state = SREG;
        cli();
        if (debug_flush_timer) {
                UENUM = DEBUG_TX_ENDPOINT;
		while ((UEINTX & (1<<RWAL))) {
			UEDATX = 0;
		}
                UEINTX = 0x3A;
                debug_flush_timer = 0;
        }
        SREG = intr_state;
}

uint32_t usb_serial_class::baud(void)
{
	return (DEBUG_TX_SIZE * 1000 / DEBUG_TX_INTERVAL);
}

uint8_t usb_serial_class::stopbits(void)
{
	return 0;
}

uint8_t usb_serial_class::paritytype(void)
{
	return 0;
}

uint8_t usb_serial_class::numbits(void)
{
	return 8;
}

uint8_t usb_serial_class::dtr(void)
{
	return 0;
}

uint8_t usb_serial_class::rts(void)
{
	return 0;
}



// Preinstantiate Objects //////////////////////////////////////////////////////

usb_serial_class	Serial = usb_serial_class();
usb_keyboard_class	Keyboard = usb_keyboard_class();


