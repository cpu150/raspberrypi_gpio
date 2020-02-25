# RASPBERRYPI_GPIO
C++ class wrapping [BCM2835](http://www.airspayce.com/mikem/bcm2835) C library which helps to manage the GPIO port of a Raspberry Pi
This class has been tested on a Raspberry Pi Zero W only

# DEPENDENCIES
## BCM2835 LIBRARY
To install the `bcm2835` library you can follow the instructions [here](http://www.airspayce.com/mikem/bcm2835)
There is a script there: `bcm2835/install.sh` which is a helper to download and install the library but may not support all OS configurations

# COMPILATION
To compile this project there is a *Makefile*. Here are the available make commands:
    - `all`   - build the project applying the *release* configuration
    - `debug` - build the project applying the *debug* configuration
    - `clean` - delete all generated files
