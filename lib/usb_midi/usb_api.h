#ifndef USBserial_h_
#define USBserial_h_

#include <inttypes.h>

#include "Print.h"

#define USB_MIDI_SYSEX_MAX 60  // maximum sysex length we can receive

class usb_midi_class
{
public:
	void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);
	void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);
	void sendPolyPressure(uint8_t note, uint8_t pressure, uint8_t channel);
	void sendControlChange(uint8_t control, uint8_t value, uint8_t channel);
	void sendProgramChange(uint8_t program, uint8_t channel);
	void sendAfterTouch(uint8_t pressure, uint8_t channel);
	void sendPitchBend(uint16_t value, uint8_t channel);
	void sendSysEx(uint8_t length, const uint8_t *data);
	void send_now(void);
	uint8_t analog2velocity(uint16_t val, uint8_t range);
	bool read(uint8_t channel=0);
	inline uint8_t getType(void) {
		return msg_type;
	};
	inline uint8_t getChannel(void) {
		return msg_channel;
	};
	inline uint8_t getData1(void) {
		return msg_data1;
	};
	inline uint8_t getData2(void) {
		return msg_data2;
	};
	inline uint8_t * getSysExArray(void) {
		return msg_sysex;
	};
	inline void setHandleNoteOff(void (*fptr)(uint8_t channel, uint8_t note, uint8_t velocity)) {
		handleNoteOff = fptr;
	};
	inline void setHandleNoteOn(void (*fptr)(uint8_t channel, uint8_t note, uint8_t velocity)) {
		handleNoteOn = fptr;
	};
	inline void setHandleVelocityChange(void (*fptr)(uint8_t channel, uint8_t note, uint8_t velocity)) {
		handleVelocityChange = fptr;
	};
	inline void setHandleControlChange(void (*fptr)(uint8_t channel, uint8_t control, uint8_t value)) {
		handleControlChange = fptr;
	};
	inline void setHandleProgramChange(void (*fptr)(uint8_t channel, uint8_t program)) {
		handleProgramChange = fptr;
	};
	inline void setHandleAfterTouch(void (*fptr)(uint8_t channel, uint8_t pressure)) {
		handleAfterTouch = fptr;
	};
	inline void setHandlePitchChange(void (*fptr)(uint8_t channel, uint16_t pitch)) {
		handlePitchChange = fptr;
	};
private:
	void send_raw(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
	void read_sysex_byte(uint8_t b);
	uint8_t msg_channel;
	uint8_t msg_type;
	uint8_t msg_data1;
	uint8_t msg_data2;
	uint8_t msg_sysex[USB_MIDI_SYSEX_MAX];
	uint8_t msg_sysex_len;
	void (*handleNoteOff)(uint8_t ch, uint8_t note, uint8_t vel);
	void (*handleNoteOn)(uint8_t ch, uint8_t note, uint8_t vel);
	void (*handleVelocityChange)(uint8_t ch, uint8_t note, uint8_t vel);
	void (*handleControlChange)(uint8_t ch, uint8_t, uint8_t);
	void (*handleProgramChange)(uint8_t ch, uint8_t);
	void (*handleAfterTouch)(uint8_t ch, uint8_t);
	void (*handlePitchChange)(uint8_t ch, uint16_t);
};

extern usb_midi_class usbMIDI;


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
