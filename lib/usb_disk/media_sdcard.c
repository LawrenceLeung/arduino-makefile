#include "WProgram.h"
#include "usb_common.h"

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


#define SPI_PORT	PORTB
#define SPI_DDR		DDRB
#define SPI_SS_PIN	0
#define SPI_SCLK_PIN	1
#define SPI_MOSI_PIN	2
#define SPI_MISO_PIN	3


#define SD_CMD_GO_IDLE_STATE		0x4095
#define SD_CMD_SEND_OP_COND		0x41FF
#define SD_CMD_SEND_IF_COND		0x4887
#define SD_CMD_SEND_CSD			0x49FF
#define SD_CMD_STOP_TRANSMISSION	0x4CFF
#define SD_CMD_READ_MULTIPLE_BLOCK	0x52FF
#define SD_CMD_WRITE_MULTIPLE_BLOCK	0x59FF
#define SD_ACMD_SEND_OP_COND		0x69FF
#define SD_CMD_APP_CMD			0x77FF
#define SD_CMD_READ_OCR			0x7AFF


inline void spi_write(uint8_t val)  __attribute__((always_inline));
inline void spi_write(uint8_t val)
{
	SPDR = val;
	while ((SPSR & (1<<SPIF)) == 0) /*wait*/ ;
}
inline uint8_t spi_xfer(uint8_t val)  __attribute__((always_inline));
inline uint8_t spi_xfer(uint8_t val)
{
	SPDR = val;
	while ((SPSR & (1<<SPIF)) == 0) /*wait*/ ;
	return SPDR;
}
void spi_ignore_bytes(uint8_t count)
{
	do {
		spi_write(0xFF);
	} while (--count);
}
inline void spi_select(void)
{
	SPI_PORT &= ~(1<<SPI_SS_PIN);
}
inline void sd_deselect(void)
{
	SPI_PORT |= (1<<SPI_SS_PIN);
	spi_write(0xFF);
}
uint8_t sd_command(uint16_t cmd, uint32_t parm)
{
	uint8_t n, r;
	union {
		struct {
			unsigned b1:8;
			unsigned b2:8;
			unsigned b3:8;
			unsigned b4:8;
		} byte;
		long i;
	} in;


	in.i = parm;

	spi_select();
	spi_write(cmd >> 8);
	spi_write(in.byte.b4);
	spi_write(in.byte.b3);
	spi_write(in.byte.b2);
	spi_write(in.byte.b1);
	SPI_PORT |= (1<<SPI_MISO_PIN);
	r = spi_xfer(cmd);
	for (n=0; n < 9; n++) {
		if (r != 0xFF) break;
		r = spi_xfer(0xFF);
	}
	SPI_PORT &= ~(1<<SPI_MISO_PIN);
	return r;
}

uint8_t sd_wait_data(void)
{
	uint16_t count=0;
	uint8_t r;

	do {
		r = spi_xfer(0xFF);
		if (r == 0xFE) return r;
	} while (count++ < 20000);
	return r;
}


#define READ_ONLY 0

#define MEDIA_PRESENT_MASK		0x10

#define MEDIA_STATE_NOCARD		0x00
#define MEDIA_STATE_INITIALIZING	0x01
#define MEDIA_STATE_READY		0x12
uint8_t media_state USBSTATE;

#define MEDIA_TYPE_SDv1			0x00
#define MEDIA_TYPE_SDv2			0x01
#define MEDIA_TYPE_SDHC			0x02
uint8_t media_type USBSTATE;

void media_restart()
{
	SPI_PORT |= (1<<SPI_SS_PIN);
	SPI_DDR |= ((1<<SPI_SS_PIN) | (1<<SPI_SCLK_PIN) | (1<<SPI_MOSI_PIN));
	SPCR = (1<<SPE) | (1<<MSTR);	// 8 Mbit/sec
	SPSR = (1<<SPI2X);
}

void media_init(void)
{
	media_restart();
	media_state = MEDIA_STATE_NOCARD;
	media_type = MEDIA_TYPE_SDv1;
	media_poll();
}

void media_poll(void)
{
	uint8_t i, r;

	if (media_state == MEDIA_STATE_NOCARD) {
		media_type = MEDIA_TYPE_SDv1;
		spi_select();
		for (i=0; i<10; i++) {
			spi_write(0xFF);
		}
		r = sd_command(SD_CMD_GO_IDLE_STATE, 0); // CMD0
		sd_deselect();
		if ((r & 0xFE) == 0) {
			r = sd_command(SD_CMD_SEND_IF_COND, 0x1AA);
			if ((r & 0xFE) == 0) {
				spi_write(0xFF);
				spi_write(0xFF);
				r = spi_xfer(0xFF);
				i = spi_xfer(0xFF);
				if (r == 1 && i == 0xAA) {
					media_type = MEDIA_TYPE_SDv2;
				}
			}
			sd_deselect();
			media_state = MEDIA_STATE_INITIALIZING;
		}
	} else {
		r = sd_command(SD_CMD_SEND_OP_COND, media_type ? 0x40000000 : 0); // CMD1
		sd_deselect();
		if (r == 0) {
			if (media_state == MEDIA_STATE_INITIALIZING && media_type) {
				r = sd_command(SD_CMD_READ_OCR, 0);
				if (r == 0) {
					r = spi_xfer(0xFF);
					if (r & 0x40) {
						media_type = MEDIA_TYPE_SDHC;
					}
					spi_write(0xFF);
					spi_write(0xFF);
					spi_write(0xFF);
				}
				sd_deselect();
			}
			media_state = MEDIA_STATE_READY;
		} else if (r & 0x80) {
			media_state = MEDIA_STATE_NOCARD;
		}
	}
	//return media_state;
}

inline uint8_t media_get_state(void)
{
	return media_state;
}

uint8_t media_lock(void)
{
	return 0;
}

void media_unlock(void)
{

}

inline uint32_t media_size(void)
{
	uint8_t r;

	if (media_state != MEDIA_STATE_READY) return 1;
	r = sd_command(SD_CMD_SEND_CSD, 0);
	if (r) goto error;
	r = sd_wait_data();
	if (r != 0xFE) goto error;
	r = spi_xfer(0xFF) & 0xC0;
	if (r == 0) {
		uint8_t c1, c2, c3, mult;
		uint16_t c_size;
		// Version 1.0 (standard capacity, up to 2 gigbytes)
		spi_ignore_bytes(4);
		mult = spi_xfer(0xFF);	// READ_BL_LEN
		mult &= 0x0F;
		c1 = spi_xfer(0xFF);	// C_SIZE[12:11]
		c2 = spi_xfer(0xFF);	// C_SIZE[10:3]
		c3 = spi_xfer(0xFF);	// C_SIZE[2:0]
		c1 &= 0x03;
		c_size = ((c1 << 10) | (c2 << 2) | (c3 >> 6)) + 1;
		c1 = spi_xfer(0xFF);	// C_SIZE_MULT[2:1]
		c2 = spi_xfer(0xFF);	// C_SIZE_MULT[0]
		c1 &= 0x03;
		c2 &= 0x80;
		c1 = (c1 << 1) | (c2 >> 7);
		mult = mult + c1 - 7;
		spi_ignore_bytes(8);
		sd_deselect();
		return ((uint32_t)c_size << mult);
	} else if (r == 0x40) {
		// Version 2.0 (high capacity, more than 2 gigbytes)
		uint8_t c1, c2, c3;
		uint32_t size;
		spi_ignore_bytes(6);
		c1 = spi_xfer(0xFF); // C_SIZE
		c2 = spi_xfer(0xFF); // C_SIZE
		c3 = spi_xfer(0xFF); // C_SIZE
		spi_ignore_bytes(9);
		c1 &= 0x3F;
		size = (((uint32_t)c1 << 16) | ((uint16_t)c2 << 8) | c3) + 1;
		size <<= 10;
		sd_deselect();
		return size;
	} else {
		goto error; // unknown CSD_STRUCTURE
	}
error:
	sd_deselect();
	media_state = MEDIA_STATE_NOCARD;
	return 1;
}


static void media_send_begin(uint32_t lba)
{
	uint8_t r;

	if (media_type != MEDIA_TYPE_SDHC) lba = (lba << 9);
	r = sd_command(SD_CMD_READ_MULTIPLE_BLOCK, lba);
	if (r) {
		// TODO: check for errors...
	}
}

static void media_send_chunk(uint32_t lba, uint8_t chunk)
{
	uint8_t i;

	if (chunk == 0) {
		i = sd_wait_data();
	}

	for (i=0; i<8; i++) {
		// TODO: asm optimization
		UEDATX = spi_xfer(0xFF);
		UEDATX = spi_xfer(0xFF);
		UEDATX = spi_xfer(0xFF);
		UEDATX = spi_xfer(0xFF);
		UEDATX = spi_xfer(0xFF);
		UEDATX = spi_xfer(0xFF);
		UEDATX = spi_xfer(0xFF);
		UEDATX = spi_xfer(0xFF);
	}
	UEINTX = 0x3A;

	if (chunk == 7) {
		spi_write(0xFF);
		spi_write(0xFF);
	}
}

static void media_send_end(void)
{
	uint8_t r;

	r = sd_command(SD_CMD_STOP_TRANSMISSION, 0);
	// TODO: proper handling of stop transaction.....
	// but what is the proper way?	Older cards stop instantly,
	// but newer ones spew 1 or 2 bytes of garbage, then maybe
	// 0xFF's until the ok (0) response.  What a mess!
	spi_write(0xFF);
	spi_write(0xFF);
	spi_write(0xFF);
	spi_write(0xFF);
	spi_write(0xFF);
	spi_write(0xFF);
	spi_write(0xFF);
	spi_write(0xFF);
	sd_deselect();
}












static void media_receive_begin(uint32_t lba)
{
	uint8_t r;

	if (media_type != MEDIA_TYPE_SDHC) lba = (lba << 9);
	sd_command(SD_CMD_WRITE_MULTIPLE_BLOCK, lba);
	if (r) {
		// TODO: check for errors...
	}
}


static void media_receive_chunk(uint32_t lba, uint8_t chunk)
{
	uint8_t i, r;

	if (chunk == 0) {
		spi_write(0xFC);
	}
	for (i=0; i<8; i++) {
		// TODO: asm optimization
		spi_write(UEDATX);
		spi_write(UEDATX);
		spi_write(UEDATX);
		spi_write(UEDATX);
		spi_write(UEDATX);
		spi_write(UEDATX);
		spi_write(UEDATX);
		spi_write(UEDATX);
	}
	UEINTX = 0x6B;

	if (chunk == 7) {
		spi_write(0xFF);
		spi_write(0xFF);
		do {
			r = spi_xfer(0xFF);
		} while (r != 0xFF);
	}
}

static void media_receive_end(void)
{
	uint8_t r;

	spi_write(0xFD);
	spi_write(0xFF);
	do {
		// TODO: this can wait for a long time... would be
		// much better to create a busy media state and
		// return from the ISR, but then need to deal with
		// it everywhere else!
		r = spi_xfer(0xFF);
	} while (r != 0xFF);

	sd_deselect();
}


