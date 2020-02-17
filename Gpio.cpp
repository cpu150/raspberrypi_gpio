#include "Gpio.h"

Gpio::Gpio(bool enable_spi) {
	if (this->init_gpio() == NO_ERROR && enable_spi) {
		this->init_spi();
	}
}

Gpio::~Gpio() {
	this->close_gpio();
}

int Gpio::init_gpio() {
  int errnum = NO_ERROR;
  // Start the BCM2835 Library to access GPIO
  if (!bcm2835_init()) {
    cerr << "bcm2835_init failed (Are you running as root?)" << endl;
    errnum = BCM2835_INIT_ERROR;
    exit(errnum);
  }
  return errnum;
}

int Gpio::init_spi() {
  int errnum = NO_ERROR;
  // Enable SPI bus
	if (!bcm2835_spi_begin()) {
		cerr << "bcm2835_spi_begin failed (Are you running as root?)" << endl;
    errnum = BCM2835_INIT_SPI_ERROR;
    exit(errnum);
	}
  return errnum;
}

int Gpio::close_gpio() {
  int errnum = NO_ERROR;
  if (!bcm2835_close()) {
    cerr << "bcm2835_close failed (Are you running as root?)" << endl;
    errnum = BCM2835_CLOSE_ERROR;
    exit(errnum);
  }
	return errnum;
}

void Gpio::setup_pwm(GpioPwmClock clock) {
	std::uint32_t clock_static = static_cast<std::uint32_t>(clock);

	bcm2835_pwm_set_clock(clock_static);
}

void Gpio::setup_pin(GpioPin pin) {
	std::uint8_t funStatic = static_cast<std::uint8_t>(pin.function);
  std::uint8_t pullUpStatic = static_cast<std::uint8_t>(pin.pullUp);
  std::uint8_t pinStatic = static_cast<std::uint8_t>(pin.name);
	Feature feature = getFeature(pin);
  LOG("set_direction(function=%d, pullUp=%d) on pin:%d", unsigned(funStatic), unsigned(pullUpStatic), unsigned(pinStatic));

	if (pin.name != GpioPinName::unknown) {
	  // Set function
		if (feature == Feature::io) {
	  	bcm2835_gpio_fsel(pinStatic, funStatic);
		} else if (feature == Feature::pwm) {
			std::uint8_t channel = get_pwm_channel(pin);
			std::uint8_t mode_static = static_cast<std::uint8_t>(pin.pwmMode);
			std::uint8_t range = pin.pwmRange;

			if (channel >= 0 && pin.pwmMode != GpioPwmMode::unknown) {
				bcm2835_pwm_set_mode(channel, mode_static, 1 /* 1=enable */);
			}

			if (channel >= 0 && range > 0) {
				bcm2835_pwm_set_range(channel, range);
			}
		}

	  // if specified set PullUp resistor
	  if (pin.pullUp != GpioPullUp::none) {
	    LOG("override pull up setting");
	    bcm2835_gpio_set_pud(pinStatic, pullUpStatic);
	  } else {
	    LOG("leave pull up setting as is");
	  }
	} else {
		cerr << "setup_pin() pin name not initialised" << endl;
	}
}

void Gpio::read(std::list<GpioPin*> pin_list) {
	for (GpioPin *pin : pin_list) {
		uint8_t pin_static = static_cast<std::uint8_t>(pin->name);
		pin->value = bcm2835_gpio_lev(pin_static);
		LOG("read pin:%d value:%d", pin_static, pin->value);
	}
}

std::list<GpioPin*> Gpio::get_pins_up(std::list<GpioPin*> pins_to_check) {
  std::list<GpioPin*> switches_pushed = {};

  if (!pins_to_check.empty()) {
    read(pins_to_check);
    for (GpioPin *pin : pins_to_check) {
      // Keep pins up only
      if ((pin->value == 0 && pin->pullUp == GpioPullUp::up) || (pin->value != 0 && pin->pullUp != GpioPullUp::up)) {
        switches_pushed.push_back(pin);
      }
    }
  }

  return switches_pushed;
}

void Gpio::write(GpioPin* pin, uint8_t val) {
	std::uint8_t val_static = static_cast<std::uint8_t>(val);
	std::uint8_t pin_static = static_cast<std::uint8_t>(pin->name);
	std::uint8_t channel = get_pwm_channel(*pin);

	if (channel < 0) {
		bcm2835_gpio_write(pin_static, val_static);
		LOG("write pin:%d value:%d", pin_static, val_static);
	} else {
		bcm2835_pwm_set_data(channel, val_static);
		LOG("write PWM pin:%d value:%d", pin_static, val_static);
	}

	pin->value = val_static;
}

std::uint8_t Gpio::get_pwm_channel(GpioPin pin) {
	std::uint8_t channel = -1;
	GpioPinName name = pin.name;
	GpioFunction func = pin.function;

	if (getFeature(pin) != Feature::pwm) {
		channel = -1;
	} else if ((name == GpioPinName::gpio13 && func == GpioFunction::alt0) ||
						 (name == GpioPinName::gpio19 && func == GpioFunction::alt5)) {
		channel = 1;
	} else if (name == GpioPinName::gpio18 && func == GpioFunction::alt5) {
		channel = 0;
	}

	return channel;
}

Feature Gpio::getFeature(GpioPin pin) {
	Feature feature = Feature::unknown;
	GpioPinName name = pin.name;
	GpioFunction func = pin.function;

		if ((name != GpioPinName::unknown && func == GpioFunction::input) ||
				(name != GpioPinName::unknown && func == GpioFunction::output)) {
					feature = Feature::io;
		} else if ((name == GpioPinName::gpio13 && func == GpioFunction::alt0) ||
							 (name == GpioPinName::gpio19 && func == GpioFunction::alt5) ||
							 (name == GpioPinName::gpio18 && func == GpioFunction::alt5)) {
					feature = Feature::pwm;
		}

	return feature;
}
