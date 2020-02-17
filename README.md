# RASPBERRYPI_GPIO
C++ class wrapping [BCM2835](http://www.airspayce.com/mikem/bcm2835) C library which helps to manage the GPIO port of a Raspberry Pi
This class has been testred on  a Raspberry Pi Zero W only

# DEPENDENCIES
## BCM2835 LIBRARY
To install `BCM2835` library see instructions [here](http://www.airspayce.com/mikem/bcm2835)
The `bcm2835/install.sh` script helps to download, compile and install the library however there is no guarantee it works on all OS

# COMPILATION
To compile this project there is a *Makefile*. Here are the available make commands:
    - `all`       - build the project with applying the release configuration
    - `debug`     - build the project with applying the debug configuration
    - `clean`     - delete all generated files
