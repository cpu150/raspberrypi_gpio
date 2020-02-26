#ifndef GPIO_CLASS_H
#define GPIO_CLASS_H

#include <string>
#include <iostream>
#include <list>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <bcm2835.h>

using namespace std;

#ifdef DEBUG
  #define LOG(...) do { if (DEBUG) { \
              printf(__VA_ARGS__); \
              printf("\n"); \
            }} while (0)
#else
  #define LOG(...)
#endif

// Error codes
#define NO_ERROR                0
#define BCM2835_INIT_ERROR      -1
#define BCM2835_INIT_SPI_ERROR  -2
#define BCM2835_CLOSE_ERROR     -3

// GPIO pin names
enum class GpioPinName: std::uint8_t {
  unknown = 0xff,
  gpio27 = RPI_V2_GPIO_P1_13,
  gpio22 = RPI_V2_GPIO_P1_15,
  gpio10 = RPI_V2_GPIO_P1_19,
  gpio19 = RPI_V2_GPIO_P1_35,
  gpio18 = RPI_V2_GPIO_P1_12,
  gpio13 = RPI_V2_GPIO_P1_33,
  gpio12 = RPI_V2_GPIO_P1_32,
  gpio4 = RPI_V2_GPIO_P1_07
};

// PWM pin names
enum class GpioPwmName {
  pwm1_gpio19,
  pwm0_gpio18,
  pwm1_gpio13,
  pwm0_gpio12
};

class GpioPwmItem {
  private:
    std::uint8_t name;
    std::uint8_t function;
    std::uint8_t channel;

    GpioPwmItem (
      std::uint8_t name,
      std::uint8_t function,
      std::uint8_t channel
    ) {
      this->name = name;
      this->function = function;
      this->channel = channel;
    }

  public:
    std::uint8_t getName() { return name; }
    std::uint8_t getFunction() { return function; }
    std::uint8_t getChannel() { return channel; }

    static GpioPwmItem* create(GpioPwmName pwm_name) {
      GpioPwmItem *item = NULL;
      switch(pwm_name) {
        case GpioPwmName::pwm1_gpio19:
          item = new GpioPwmItem(RPI_V2_GPIO_P1_35, BCM2835_GPIO_FSEL_ALT5, 1);
        break;

        case GpioPwmName::pwm0_gpio18:
        item = new GpioPwmItem(RPI_V2_GPIO_P1_12, BCM2835_GPIO_FSEL_ALT5, 0);
        break;

        case GpioPwmName::pwm1_gpio13:
        item = new GpioPwmItem(RPI_V2_GPIO_P1_33, BCM2835_GPIO_FSEL_ALT0, 1);
        break;

        case GpioPwmName::pwm0_gpio12:
        item = new GpioPwmItem(RPI_V2_GPIO_P1_32, BCM2835_GPIO_FSEL_ALT0, 0);
        break;
      }
      return item;
    }
};

// Functions
enum class GpioFunction: std::uint8_t {
  unknown = 0xff,
  input = BCM2835_GPIO_FSEL_INPT,
  output = BCM2835_GPIO_FSEL_OUTP
};

// Pull up resistor
enum class GpioPullUp: std::uint8_t {
  none = 0xff,
  off = BCM2835_GPIO_PUD_OFF,
  up = BCM2835_GPIO_PUD_UP,
  down = BCM2835_GPIO_PUD_DOWN
};

// PWM clock values
enum class GpioPwmClock: std::uint32_t {
  KHZ_4_6875 = BCM2835_PWM_CLOCK_DIVIDER_1,    /*!< 1 = 4.6875kHz, same as divider 4096 */
  KHZ_9_375 = BCM2835_PWM_CLOCK_DIVIDER_2048,  /*!< 2048 = 9.375kHz */
  KHZ_18_75 = BCM2835_PWM_CLOCK_DIVIDER_1024,  /*!< 1024 = 18.75kHz */
  KHZ_37_5 = BCM2835_PWM_CLOCK_DIVIDER_512,    /*!< 512 = 37.5kHz */
  KHZ_75 = BCM2835_PWM_CLOCK_DIVIDER_256,      /*!< 256 = 75kHz */
  KHZ_150 = BCM2835_PWM_CLOCK_DIVIDER_128,     /*!< 128 = 150kHz */
  KHZ_300 = BCM2835_PWM_CLOCK_DIVIDER_64,      /*!< 64 = 300kHz */
  KHZ_600 = BCM2835_PWM_CLOCK_DIVIDER_32,      /*!< 32 = 600.0kHz */
  MHZ_1_2 = BCM2835_PWM_CLOCK_DIVIDER_16,      /*!< 16 = 1.2MHz */
  MHZ_2_4 = BCM2835_PWM_CLOCK_DIVIDER_8,       /*!< 8 = 2.4MHz */
  MHZ_4_8 = BCM2835_PWM_CLOCK_DIVIDER_4,       /*!< 4 = 4.8MHz */
  MHZ_9_6 = BCM2835_PWM_CLOCK_DIVIDER_2        /*!< 2 = 9.6MHz, fastest you can get */
};

// PWM modes
enum class GpioPwmMode: std::uint8_t {
  unknown = 0xff,
  balanced = 0,
  markspace = 1
};

// GPIO
class GpioPinIO
{
private:
  GpioPinName name      = GpioPinName::unknown;
  bool value            = false;
  GpioFunction function = GpioFunction::unknown;
  GpioPullUp pullUp     = GpioPullUp::none;

  public:
    GpioPinName getName() { return name; }
    bool getValue() { return value; }
    void setValue(bool value) { this->value = value; }
    GpioFunction getFunction() { return function; }
    GpioPullUp getPullUp() { return pullUp; }

    GpioPinIO (
      GpioPinName name,
      GpioFunction function = GpioFunction::unknown,
      GpioPullUp pullUp     = GpioPullUp::none
    ) {
      this->name = name;
      this->function = function;
      this->pullUp = pullUp;
    }
};

// PWM
class GpioPwm
{
  private:
    GpioPwmItem *item   = NULL;
    std::uint32_t value = 0;
    GpioPwmMode mode    = GpioPwmMode::unknown;
    std::uint32_t range = 0xff;

  public:
    GpioPwmItem* getItem() { return item; }
    std::uint32_t getValue() { return value; }
    void setValue(std::uint32_t value) { this->value = value; }
    GpioPwmMode getMode() { return mode; }
    std::uint32_t getRange() { return range; }

    GpioPwm (
      GpioPwmItem *item,
      std::uint32_t range = 0xff,
      GpioPwmMode mode    = GpioPwmMode::unknown
    ) {
      this->item = item;
      this->range = range;
      this->mode = mode;
    }
};

class Gpio {
  public:
    Gpio(bool enable_spi = false);
    ~Gpio();

    void setup_pin(GpioPinIO pin);
    void setup_pin(GpioPwm pin);
    void setup_pwm(GpioPwmClock clock);

    void write(GpioPinIO* pin, bool val);
    void write(GpioPwm* pin, std::uint32_t val);

    std::list<GpioPinIO*> get_pins_up(std::list<GpioPinIO*> pins_to_check);
    void read(std::list<GpioPinIO*> pin_list);
    void read(std::list<GpioPwm*> pin_list);

  private:
    bool enable_spi = false;
    int init_gpio();
    int init_spi();
    int close_gpio();
};

#endif
