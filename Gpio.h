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

// Pin names
enum class GpioPinName: std::uint8_t {
  unknown = 0xff,
  gpio27 = RPI_V2_GPIO_P1_13,
  gpio22 = RPI_V2_GPIO_P1_15,
  gpio10 = RPI_V2_GPIO_P1_19,
  gpio19 = RPI_V2_GPIO_P1_35,   /*!< Can be PWM channel 1 in ALT FUN 5 */
  gpio18 = RPI_V2_GPIO_P1_12,   /*!< Can be PWM channel 0 in ALT FUN 5 */
  gpio13 = RPI_V2_GPIO_P1_33,   /*!< Can be PWM channel 1 in ALT FUN 0 */
  gpio12 = RPI_V2_GPIO_P1_32,   /*!< Can be PWM channel 0 in ALT FUN 0 */
  gpio4 = RPI_V2_GPIO_P1_07
};

// Functions
enum class GpioFunction: std::uint8_t {
  unknown = 0xff,
  input = BCM2835_GPIO_FSEL_INPT,
  output = BCM2835_GPIO_FSEL_OUTP,
  alt0 = BCM2835_GPIO_FSEL_ALT0,
  alt1 = BCM2835_GPIO_FSEL_ALT1,
  alt2 = BCM2835_GPIO_FSEL_ALT2,
  alt3 = BCM2835_GPIO_FSEL_ALT3,
  alt4 = BCM2835_GPIO_FSEL_ALT4,
  alt5 = BCM2835_GPIO_FSEL_ALT5
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

// GPIO pin
struct GpioPin {
  GpioPinName name        = GpioPinName::unknown;
  GpioFunction function   = GpioFunction::unknown;
  GpioPullUp pullUp       = GpioPullUp::none;

  // PWM
  GpioPwmMode pwmMode     = GpioPwmMode::unknown;
  std::uint32_t pwmRange  = 255;

  std::uint32_t value     = 0;
};

enum class Feature {
  unknown,
  io,
  pwm,
  spi
};

class Gpio {
  public:
    Gpio(bool enable_spi = false);
    ~Gpio();

    void setup_pin(GpioPin pin);
    void setup_pwm(GpioPwmClock clock);
    void read(std::list<GpioPin*> pin_list);
    std::list<GpioPin*> get_pins_up(std::list<GpioPin*> pins_to_check);
    void write(GpioPin* pin, std::uint8_t val);

  private:
    int init_gpio();
  	int init_spi();
  	int close_gpio();
    std::uint8_t get_pwm_channel(GpioPin pin);
    Feature getFeature(GpioPin pin);
};

#endif
