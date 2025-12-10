#include "Adafruit_I2CDeviceV.h"

#ifndef Adafruit_I2CRegisterV_h
#define Adafruit_I2CRegisterV_h

class Adafruit_I2CRegister_
{
public:
  Adafruit_I2CRegister_(Adafruit_I2CDevice_ *device, uint16_t reg_addr,
                       uint8_t width = 1, uint8_t bitorder = LSBFIRST,
                       uint8_t address_width = 1);

  bool read(uint8_t *buffer, uint8_t len);
  bool read(uint8_t *value);
  bool read(uint16_t *value);
  bool read(uint32_t *value);
  uint32_t read(void);
  bool write(uint8_t *buffer, uint8_t len);
  bool write(uint32_t value, uint8_t numbytes = 0);

  uint8_t width(void) { return _width; }

  void print(Stream *s = &Serial);
  void println(Stream *s = &Serial);

private:
  Adafruit_I2CDevice_ *_device;
  uint16_t _address;
  uint8_t _width, _addrwidth, _bitorder;
  uint8_t _buffer[4]; // we wont support anything larger than uint32 for non-buffered read
};

class Adafruit_I2CRegisterBits_
{
public:
  Adafruit_I2CRegisterBits_(Adafruit_I2CRegister_ *reg, uint8_t bits, uint8_t shift);
  void write(uint32_t value);
  uint32_t read(void);

private:
  Adafruit_I2CRegister_ *_register;
  uint8_t _bits, _shift;
};

#endif //I2CRegister_h