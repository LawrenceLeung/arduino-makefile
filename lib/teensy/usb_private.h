#if defined(USB_SERIAL)
#include "../usb_serial/usb_private.h"
#elif defined(USB_HID)
#include "../usb_hid/usb_private.h"
#elif defined(USB_DISK) || defined(USB_DISK_SDFLASH)
#include "../usb_disk/usb_private.h"
#elif defined(USB_MIDI)
#include "../usb_midi/usb_private.h"
#endif
