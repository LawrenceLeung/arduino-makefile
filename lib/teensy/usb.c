#if defined(USB_SERIAL)
#include "../usb_serial/usb.c"
#elif defined(USB_HID)
#include "../usb_hid/usb.c"
#elif defined(USB_DISK) || defined(USB_DISK_SDFLASH)
#include "../usb_disk/usb.c"
#elif defined(USB_MIDI)
#include "../usb_midi/usb.c"
#endif
