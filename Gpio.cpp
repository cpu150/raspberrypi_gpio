#include "Gpio.h"

Gpio::Gpio(bool enable_spi) {
	this->enable_spi = enable_spi;

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
		std::cerr << "bcm2835_init failed (Are you running as root?)" << std::endl;
		errnum = BCM2835_INIT_ERROR;
		exit(errnum);
	}
	return errnum;
}

int Gpio::init_spi() {
	int errnum = NO_ERROR;
	// Enable SPI bus
	if (!bcm2835_spi_begin()) {
		std::cerr << "bcm2835_spi_begin failed (Are you running as root?)" << std::endl;
		errnum = BCM2835_INIT_SPI_ERROR;
		exit(errnum);
	}
	return errnum;
}

int Gpio::close_gpio() {
	int errnum = NO_ERROR;

	if (enable_spi) {
		bcm2835_spi_end();
	}

	if (!bcm2835_close()) {
		std::cerr << "bcm2835_close failed (Are you running as root?)" << std::endl;
		errnum = BCM2835_CLOSE_ERROR;
		exit(errnum);
	}

	return errnum;
}

void Gpio::setup_pwm(GpioPwmClock clock) {
	std::uint32_t clock_static = static_cast<std::uint32_t>(clock);
	bcm2835_pwm_set_clock(clock_static);
	LOG("set pwm clock:%d", clock_static);
}

void Gpio::setup_pin(GpioPinIO pin) {
	std::uint8_t pin_static = static_cast<std::uint8_t>(pin.getName());
	std::uint8_t fun_static = static_cast<std::uint8_t>(pin.getFunction());

	bcm2835_gpio_fsel(pin_static, fun_static);
	LOG("setup io pin:%d function:%d", pin_static, fun_static);

	// if specified set PullUp resistor
	if (pin.getPullUp() != GpioPullUp::none) {
		LOG("override pull up setting");
		bcm2835_gpio_set_pud(pin_static, static_cast<std::uint8_t>(pin.getPullUp()));
	} else {
		LOG("leave pull up setting as is");
	}
}

void Gpio::setup_pin(GpioPwm pwm) {
	GpioPwmItem *item = pwm.getItem();
	if (item != NULL) {
		std::uint8_t pin = item->getName();
		std::uint8_t channel = item->getChannel();
		std::uint8_t function = item->getFunction();

		bcm2835_gpio_fsel(pin, function);
		LOG("setup pwm pin:%d function:%d", pin, function);

		if (channel >= 0 && pwm.getMode() != GpioPwmMode::unknown) {
			bcm2835_pwm_set_mode(channel, static_cast<std::uint8_t>(pwm.getMode()), 1 /* 1=enable */);
		}

		if (channel >= 0 && pwm.getRange() > 0) {
			bcm2835_pwm_set_range(channel, pwm.getRange());
		}
	}
}

void Gpio::read(std::list<GpioPinIO*> pin_list) {
	for (GpioPinIO *pin : pin_list) {
		std::uint8_t pin_static = static_cast<std::uint8_t>(pin->getName());

		pin->setValue(bcm2835_gpio_lev(pin_static) != 0);
		LOG("read IO pin:%d value:%d", pin_static, pin->getValue());
	}
}

void Gpio::read(std::list<GpioPwm*> pin_list) {
	for (GpioPwm *pin : pin_list) {
		GpioPwmItem *item = pin->getItem();
		if (item != NULL) {
			pin->setValue(bcm2835_gpio_lev(item->getName()));
			LOG("read PWM pin:%d value:%d", item->getName(), pin->getValue());
		} else {
			LOG("read PWM error: pin not initialized");
		}
	}
}

std::list<GpioPinIO*> Gpio::get_pins_up(std::list<GpioPinIO*> pins_to_check) {
	std::list<GpioPinIO*> switches_pushed = {};

	if (!pins_to_check.empty()) {
		read(pins_to_check);

		for (GpioPinIO *pin : pins_to_check) {
			// Keep pins up only
			if ((!pin->getValue() && pin->getPullUp() == GpioPullUp::up) ||
					(pin->getValue() && pin->getPullUp() != GpioPullUp::up)) {
				switches_pushed.push_back(pin);
			}
		}
	}

	return switches_pushed;
}

void Gpio::write(GpioPinIO* pin, bool val) {
	std::uint8_t pin_static = static_cast<std::uint8_t>(pin->getName());
	std::uint8_t val_static = static_cast<std::uint8_t>(val);

	bcm2835_gpio_write(pin_static, val_static);
	LOG("write pin:%d value:%d", pin_static, val_static);
}

void Gpio::write(GpioPwm* pin, std::uint32_t val) {
	GpioPwmItem *item = pin->getItem();
	if (item != NULL) {
		bcm2835_pwm_set_data(item->getChannel(), val);
		LOG("write PWM channel:%d value:%d", item->getChannel(), val);
	} else {
		std::cerr << "write PWM channel not found" << std::endl;
	}
}
