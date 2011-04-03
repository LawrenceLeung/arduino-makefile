#include <WProgram.h>
#include "usb_private.h"
//#define USB_SERIAL_PRIVATE_INCLUDE
//#include "usb_mass_storage_debug.h"
//#include "flash.h"

//#include "pauls_ugly_debug.h"


inline uint32_t media_size(void);
void media_init(void);
void media_poll(void);
inline uint8_t media_get_state(void);
uint8_t media_lock(void);
void media_unlock(void);
static void media_send_begin(uint32_t lba);
static void media_send_chunk(uint32_t lba, uint8_t chunk);
static void media_send_end(void);
static void media_receive_begin(uint32_t lba);
static void media_receive_chunk(uint32_t lba, uint8_t chunk);
static void media_receive_end(void);



/**************************************************************************
 *
 *  Storage Media Access Functions
 *
 **************************************************************************/

void media_restart(void)
{
}

void media_init(void)
{
}

void media_poll(void)
{
}


// return the number of 512 byte blocks, or 0 if the media is offline
//
inline uint32_t media_size(void)
{
	uint8_t r;
	asm volatile(
		"ldi %0, %1"				"\n\t"
		"subi %0, hi8(pm(__data_load_end))"	"\n"
		: "=d" (r) : "M" (EPZ - 1));
	return r;
}

// __data_load_start = 07A8
// end = 7E00, FC00, 1FC00

// pm(__data_load_start) = 3D4
// end = 3F00, 7E00, FE00

static void media_send_begin(uint32_t lba)
{
}

static inline void media_send_chunk(uint32_t lba, uint8_t chunk)
{
	uint8_t addr = lba;
	asm volatile(
		"ldi	r31, %1"			"\n\t"
		"sub	r31, %3"			"\n\t"
		"lsl	r31"				"\n\t"
#if defined(__AVR_AT90USB1286__)
		"adc	__zero_reg__, __zero_reg__"	"\n\t"
		"out	0x3B, __zero_reg__"		"\n\t"
		"clr	__zero_reg__"			"\n\t"
#endif
		"mov	r30, %0"			"\n\t"
		"andi	r30, 7"				"\n\t"
		"swap	r30"				"\n\t"
		"lsl	r30"				"\n\t"
		"lsl	r30"				"\n\t"
		"adc	r31, __zero_reg__"		"\n\t"
		"ldi	%0, 8"				"\n\t"
	"L_%=_looop:"					"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		LPM	"__tmp_reg__, Z+"		"\n\t"
		"st	X, __tmp_reg__"			"\n\t"
		"subi	%0, 1"				"\n\t"
		"brne	L_%=_looop"			"\n\t"
		: "+d" (chunk)
		: "M" (EPZ - 1), "x" (&UEDATX), "r" (addr)
		: "r0", "r30", "r31"
	);
	UEINTX = 0x3A;
}


static void media_send_end(void)
{
}

static void media_receive_begin(uint32_t lba)
{
}

static void media_receive_chunk(uint32_t lba, uint8_t chunk)
{
#if defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
	uint8_t addr = lba;
	uint8_t i;
	uint8_t aa;
	uint32_t a;

	aa = chunk << 6;
	for (i=0; i<32; i++) {
		uint16_t tmp;
		tmp = UEDATX;
		tmp |= (UEDATX << 8);
		boot_flash_fill_temp_buffer(tmp, aa);
		aa += 2;
	}
	UEINTX = 0x6B;
	if ((chunk & 0x03) == 3) {
		a = ((uint32_t)(EPZ - 1 - addr) << 9) | ((uint16_t)(chunk & 4) << 6);
		asm volatile("clr r0 \n\tcom r0\n\tmov r1, r0");
		boot_flash_page_erase_and_write(a);
		asm volatile("clr __zero_reg__");
	}
#else
	UEINTX = 0x6B;
#endif
}

static void media_receive_end(void)
{
}



#define MEDIA_PRESENT_MASK		0x10

inline uint8_t media_get_state(void)
{
	return MEDIA_PRESENT_MASK;
}








