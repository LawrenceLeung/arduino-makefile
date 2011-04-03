#ifndef USBserial_h_
#define USBserial_h_

#include <inttypes.h>

#include "keylayouts.h"
#include "Print.h"

class usb_keyboard_class : public Print
{
	public:
	virtual void write(uint8_t);
	void set_modifier(uint8_t);
	void set_key1(uint8_t);
	void set_key2(uint8_t);
	void set_key3(uint8_t);
	void set_key4(uint8_t);
	void set_key5(uint8_t);
	void set_key6(uint8_t);
	void send_now(void);
};

extern usb_keyboard_class Keyboard;


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
