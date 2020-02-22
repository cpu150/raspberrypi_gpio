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
	std::uint8_t pin_static = static_cast<std::uint8_t>(pin.name);
	std::uint8_t fun_static = static_cast<std::uint8_t>(pin.function);

	bcm2835_gpio_fsel(pin_static, fun_static);
	LOG("setup pin:%d function:%d", pin_static, fun_static);

	// if specified set PullUp resistor
	if (pin.pullUp != GpioPullUp::none) {
		LOG("override pull up setting");
		bcm2835_gpio_set_pud(pin_static, static_cast<std::uint8_t>(pin.pullUp));
	} else {
		LOG("leave pull up setting as is");
	}
}

void Gpio::setup_pin(GpioPwm pin) {
	std::uint8_t channel = get_pwm_channel(pin);

	if (channel >= 0 && pin.mode != GpioPwmMode::unknown) {
		bcm2835_pwm_set_mode(channel, static_cast<std::uint8_t>(pin.mode), 1 /* 1=enable */);
	}

	if (channel >= 0 && pin.range > 0) {
		bcm2835_pwm_set_range(channel, pin.range);
	}
}

std::uint8_t Gpio::get_pwm_channel(GpioPwm pin) {
	std::uint8_t channel = -1;

	switch(pin.name) {
		case GpioPwmName::pwm1_gpio19:
		case GpioPwmName::pwm1_gpio13:
			channel = 1;
			break;

		case GpioPwmName::pwm0_gpio18:
		case GpioPwmName::pwm0_gpio12:
			channel = 0;
			break;

		default:
			channel = -1;
			break;
	}

	return channel;
}

void Gpio::read(std::list<GpioPinIO*> pin_list) {
	for (GpioPinIO *pin : pin_list) {
		std::uint8_t pin_static = static_cast<std::uint8_t>(pin->name);

		pin->value = bcm2835_gpio_lev(pin_static) != 0;
		LOG("read IO pin:%d value:%d", pin_static, pin->value);
	}
}

void Gpio::read(std::list<GpioPwm*> pin_list) {
	for (GpioPwm *pin : pin_list) {
		std::uint8_t pin_static = static_cast<std::uint8_t>(pin->name);

		pin->value = bcm2835_gpio_lev(pin_static);
		LOG("read PWM pin:%d value:%d", pin_static, pin->value);
	}
}

std::list<GpioPinIO*> Gpio::get_pins_up(std::list<GpioPinIO*> pins_to_check) {
	std::list<GpioPinIO*> switches_pushed = {};

	if (!pins_to_check.empty()) {
		read(pins_to_check);

		for (GpioPinIO *pin : pins_to_check) {
			// Keep pins up only
			if ((!pin->value && pin->pullUp == GpioPullUp::up) ||
					(pin->value && pin->pullUp != GpioPullUp::up)) {
				switches_pushed.push_back(pin);
			}
		}
	}

	return switches_pushed;
}

void Gpio::write(GpioPinIO* pin, bool val) {
	std::uint8_t pin_static = static_cast<std::uint8_t>(pin->name);
	std::uint8_t val_static = static_cast<std::uint8_t>(val);

	bcm2835_gpio_write(pin_static, val_static);
	LOG("write pin:%d value:%d", pin_static, val_static);
}

void Gpio::write(GpioPwm* pin, std::uint32_t val) {
	std::uint8_t channel = get_pwm_channel(*pin);

	if (channel >= 0) {
		bcm2835_pwm_set_data(channel, val);
		LOG("write PWM channel:%d value:%d", channel, val);
	} else {
		std::cerr << "write PWM channel not found" << std::endl;
	}
}
