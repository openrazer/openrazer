#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#ifdef __cplusplus
class UsbDevice
{
public:
    static HANDLE OpenDevice(unsigned short vendor, unsigned short product, unsigned int MI);
    static bool SendToDevice(HANDLE handle, unsigned char* data, unsigned int length);
};
#else
  typedef
    struct UsbDevice
      UsbDevice;
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern HANDLE call_UsbDevice_OpenDevice(unsigned short vendor, unsigned short product, unsigned int MI);
extern int call_UsbDevice_SendToDevice(HANDLE handle, unsigned char* data, unsigned int length);

#ifdef __cplusplus
}
#endif
#endif
