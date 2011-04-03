#ifndef USBserial_h_
#define USBserial_h_

#include <inttypes.h>

#include "keylayouts.h"
#include "Print.h"

class usb_keyboard_class : public Print
{
	public:
	virtual void write(uint8_t);
	void write_unicode(uint16_t unicode_code_point);
	void set_modifier(uint8_t);
	void set_key1(uint8_t);
	void set_key2(uint8_t);
	void set_key3(uint8_t);
	void set_key4(uint8_t);
	void set_key5(uint8_t);
	void set_key6(uint8_t);
	void send_now(void);
	private:
	void write_keycode(KEYCODE_TYPE key);
	void write_key(uint8_t code);
	uint8_t utf8_state;
	uint16_t unicode_wchar;
};

extern usb_keyboard_class Keyboard;



class usb_mouse_class
{
	public:
	void move(int8_t x, int8_t y, int8_t wheel=0);
	void click(void);
	void scroll(int8_t wheel);
	void set_buttons(uint8_t left, uint8_t middle=0, uint8_t right=0);
};

extern usb_mouse_class Mouse;


class usb_serial_class : public Print
{
	public:
	// standard Arduino functions
	void begin(long);
	void end(void);
	uint8_t available(void);
	int read(void);
	void flush(void);
	virtual void write(uint8_t);
	// Teensy extensions
	void send_now(void);
	uint32_t baud(void);
	uint8_t stopbits(void);
	uint8_t paritytype(void);
	uint8_t numbits(void);
	uint8_t dtr(void);
	uint8_t rts(void);
	private:
	uint8_t readnext(void);
};

extern usb_serial_class Serial;


#endif
