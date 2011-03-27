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


static const KEYCODE_TYPE PROGMEM keycodes_ascii[] = {
	ASCII_20, ASCII_21, ASCII_22, ASCII_23,
	ASCII_24, ASCII_25, ASCII_26, ASCII_27,
	ASCII_28, ASCII_29, ASCII_2A, ASCII_2B,
	ASCII_2C, ASCII_2D, ASCII_2E, ASCII_2F,
	ASCII_30, ASCII_31, ASCII_32, ASCII_33,
	ASCII_34, ASCII_35, ASCII_36, ASCII_37,
	ASCII_38, ASCII_39, ASCII_3A, ASCII_3B,
	ASCII_3C, ASCII_3D, ASCII_3E, ASCII_3F,
	ASCII_40, ASCII_41, ASCII_42, ASCII_43,
	ASCII_44, ASCII_45, ASCII_46, ASCII_47,
	ASCII_48, ASCII_49, ASCII_4A, ASCII_4B,
	ASCII_4C, ASCII_4D, ASCII_4E, ASCII_4F,
	ASCII_50, ASCII_51, ASCII_52, ASCII_53,
	ASCII_54, ASCII_55, ASCII_56, ASCII_57,
	ASCII_58, ASCII_59, ASCII_5A, ASCII_5B,
	ASCII_5C, ASCII_5D, ASCII_5E, ASCII_5F,
	ASCII_60, ASCII_61, ASCII_62, ASCII_63,
	ASCII_64, ASCII_65, ASCII_66, ASCII_67,
	ASCII_68, ASCII_69, ASCII_6A, ASCII_6B,
	ASCII_6C, ASCII_6D, ASCII_6E, ASCII_6F,
	ASCII_70, ASCII_71, ASCII_72, ASCII_73,
	ASCII_74, ASCII_75, ASCII_76, ASCII_77,
	ASCII_78, ASCII_79, ASCII_7A, ASCII_7B,
	ASCII_7C, ASCII_7D, ASCII_7E, ASCII_7F
};
#ifdef ISO_8859_1_A0
static const KEYCODE_TYPE PROGMEM keycodes_iso_8859_1[] = {
	ISO_8859_1_A0, ISO_8859_1_A1, ISO_8859_1_A2, ISO_8859_1_A3,
	ISO_8859_1_A4, ISO_8859_1_A5, ISO_8859_1_A6, ISO_8859_1_A7,
	ISO_8859_1_A8, ISO_8859_1_A9, ISO_8859_1_AA, ISO_8859_1_AB,
	ISO_8859_1_AC, ISO_8859_1_AD, ISO_8859_1_AE, ISO_8859_1_AF,
	ISO_8859_1_B0, ISO_8859_1_B1, ISO_8859_1_B2, ISO_8859_1_B3,
	ISO_8859_1_B4, ISO_8859_1_B5, ISO_8859_1_B6, ISO_8859_1_B7,
	ISO_8859_1_B8, ISO_8859_1_B9, ISO_8859_1_BA, ISO_8859_1_BB,
	ISO_8859_1_BC, ISO_8859_1_BD, ISO_8859_1_BE, ISO_8859_1_BF,
	ISO_8859_1_C0, ISO_8859_1_C1, ISO_8859_1_C2, ISO_8859_1_C3,
	ISO_8859_1_C4, ISO_8859_1_C5, ISO_8859_1_C6, ISO_8859_1_C7,
	ISO_8859_1_C8, ISO_8859_1_C9, ISO_8859_1_CA, ISO_8859_1_CB,
	ISO_8859_1_CC, ISO_8859_1_CD, ISO_8859_1_CE, ISO_8859_1_CF,
	ISO_8859_1_D0, ISO_8859_1_D1, ISO_8859_1_D2, ISO_8859_1_D3,
	ISO_8859_1_D4, ISO_8859_1_D5, ISO_8859_1_D6, ISO_8859_1_D7,
	ISO_8859_1_D8, ISO_8859_1_D9, ISO_8859_1_DA, ISO_8859_1_DB,
	ISO_8859_1_DC, ISO_8859_1_DD, ISO_8859_1_DE, ISO_8859_1_DF,
	ISO_8859_1_E0, ISO_8859_1_E1, ISO_8859_1_E2, ISO_8859_1_E3,
	ISO_8859_1_E4, ISO_8859_1_E5, ISO_8859_1_E6, ISO_8859_1_E7,
	ISO_8859_1_E8, ISO_8859_1_E9, ISO_8859_1_EA, ISO_8859_1_EB,
	ISO_8859_1_EC, ISO_8859_1_ED, ISO_8859_1_EE, ISO_8859_1_EF,
	ISO_8859_1_F0, ISO_8859_1_F1, ISO_8859_1_F2, ISO_8859_1_F3,
	ISO_8859_1_F4, ISO_8859_1_F5, ISO_8859_1_F6, ISO_8859_1_F7,
	ISO_8859_1_F8, ISO_8859_1_F9, ISO_8859_1_FA, ISO_8859_1_FB,
	ISO_8859_1_FC, ISO_8859_1_FD, ISO_8859_1_FE, ISO_8859_1_FF
};
#endif // ISO_8859_1_A0


// Step #1, decode UTF8 to Unicode code points
//
void usb_keyboard_class::write(uint8_t c)
{
	if (c < 0x80) {
		// single byte encoded, 0x00 to 0x7F
		utf8_state = 0;
		write_unicode(c);
	} else if (c < 0xC0) {
		// 2nd, 3rd or 4th byte, 0x80 to 0xBF
		c &= 0x3F;
		if (utf8_state == 1) {
			utf8_state = 0;
			write_unicode(unicode_wchar | c);
		} else if (utf8_state == 2) {
			unicode_wchar |= ((uint16_t)c << 6);
			utf8_state = 1;
		}
	} else if (c < 0xE0) {
		// begin 2 byte sequence, 0xC2 to 0xDF
		// or illegal 2 byte sequence, 0xC0 to 0xC1
		unicode_wchar = (uint16_t)(c & 0x1F) << 6;
		utf8_state = 1;
	} else if (c < 0xF0) {
		// begin 3 byte sequence, 0xE0 to 0xEF
		unicode_wchar = (uint16_t)(c & 0x0F) << 12;
		utf8_state = 2;
	} else {
		// begin 4 byte sequence (not supported), 0xF0 to 0xF4
		// or illegal, 0xF5 to 0xFF
		utf8_state = 255;
	}
}


// Step #2: translate Unicode code point to keystroke sequence
//
void usb_keyboard_class::write_unicode(uint16_t cpoint)
{
	// Unicode code points beyond U+FFFF are not supported
	// technically this input should probably be called UCS-2
	if (cpoint < 32) {
		if (cpoint == 10) write_key(KEY_ENTER);
		return;
	}
	if (cpoint < 128) {
		if (sizeof(KEYCODE_TYPE) == 1) {
			write_keycode(pgm_read_byte(keycodes_ascii + (cpoint - 0x20)));
		} else if (sizeof(KEYCODE_TYPE) == 2) {
			write_keycode(pgm_read_word(keycodes_ascii + (cpoint - 0x20)));
		}
		return;
	}
	#ifdef ISO_8859_1_A0
	if (cpoint <= 0xA0) return;
	if (cpoint < 0x100) {
		if (sizeof(KEYCODE_TYPE) == 1) {
			write_keycode(pgm_read_byte(keycodes_iso_8859_1 + (cpoint - 0xA0)));
		} else if (sizeof(KEYCODE_TYPE) == 2) {
			write_keycode(pgm_read_word(keycodes_iso_8859_1 + (cpoint - 0xA0)));
		}
		return;
	}
	#endif
	#ifdef UNICODE_20AC
		if (cpoint == 0x20AC) write_keycode(UNICODE_20AC);
	#endif
}

// Step #3: execute keystroke sequence
//
void usb_keyboard_class::write_keycode(KEYCODE_TYPE key)
{
	if (!key) return;
	#ifdef DEADKEYS_MASK
	KEYCODE_TYPE deadkey = key & DEADKEYS_MASK;
	#ifdef ACUTE_ACCENT_BITS
	if (deadkey == ACUTE_ACCENT_BITS) write_key(DEADKEY_ACUTE_ACCENT);
	#endif
	#ifdef CEDILLA_BITS
	if (deadkey == CEDILLA_BITS) write_key(DEADKEY_CEDILLA);
	#endif
	#ifdef CIRCUMFLEX_BITS
	if (deadkey == CIRCUMFLEX_BITS) write_key(DEADKEY_CIRCUMFLEX);
	#endif
	#ifdef DIAERESIS_BITS
	if (deadkey == DIAERESIS_BITS) write_key(DEADKEY_DIAERESIS);
	#endif
	#ifdef GRAVE_ACCENT_BITS
	if (deadkey == GRAVE_ACCENT_BITS) write_key(DEADKEY_GRAVE_ACCENT);
	#endif
	#ifdef TILDE_BITS
	if (deadkey == TILDE_BITS) write_key(DEADKEY_TILDE);
	#endif
	#endif // DEADKEYS_MASK
	write_key(key);
}

// Step #4: do each keystroke
//
void usb_keyboard_class::write_key(uint8_t key)
{
	uint8_t modifier=0;

	#ifdef SHIFT_MASK
	if (key & SHIFT_MASK) modifier |= MODIFIERKEY_SHIFT;
	#endif
	#ifdef ALTGR_MASK
	if (key & ALTGR_MASK) modifier |= MODIFIERKEY_RIGHT_ALT;
	#endif
	keyboard_modifier_keys = modifier;
	key &= 0x3F;
	#ifdef KEY_NON_US_100
	if (key == KEY_NON_US_100) key = 100;
	#endif
	keyboard_keys[0] = key;
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






void usb_mouse_class::move(int8_t x, int8_t y, int8_t wheel)
{
        uint8_t intr_state, timeout;

        if (!usb_configuration) return;
        if (x == -128) x = -127;
        if (y == -128) y = -127;
        if (wheel == -128) wheel = -127;
        intr_state = SREG;
        cli();
        UENUM = MOUSE_ENDPOINT;
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
                UENUM = MOUSE_ENDPOINT;
        }
        UEDATX = mouse_buttons;
        UEDATX = x;
        UEDATX = y;
        UEDATX = wheel;
        UEINTX = 0x3A;
        SREG = intr_state;
}

void usb_mouse_class::click(void)
{
	mouse_buttons = 1;
	move(0, 0);
	mouse_buttons = 0;
	move(0, 0);
}

void usb_mouse_class::scroll(int8_t wheel)
{
	move(0, 0, wheel);
}

void usb_mouse_class::set_buttons(uint8_t left, uint8_t middle, uint8_t right)
{
        uint8_t mask=0;

        if (left) mask |= 1;
        if (middle) mask |= 4;
        if (right) mask |= 2;
        mouse_buttons = mask;
        move(0, 0);
}



void usb_serial_class::begin(long speed)
{
	// make sure USB is initialized
	usb_init();
	uint16_t begin_wait = (uint16_t)millis();
	while (1) {
		if (usb_configuration) {
			delay(200);  // a little time for host to load a driver
			return;
		}
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
	// this seems to be causig a lockup... why????
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
usb_mouse_class		Mouse = usb_mouse_class();


