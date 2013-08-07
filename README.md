### CrazyFlie Firmware

Crazyflie is a tiny quadcopter. See more details on the [BitCraze website](http://www.bitcraze.se/),
[the product page](http://www.seeedstudio.com/depot/bitcraze-m-64.html)
or just enjoy [Crazyflie video footage](http://www.youtube.com/watch?v=3WBUVYZkODI).

This repository contains a modification of Crazyflie firmware.
The official firmware repository is http://bitbucket.org/bitcraze/crazyflie-firmware.

![Crazyflie](https://raw.github.com/samofly/crazyflie-firmware/krasin/res/crazyflie.jpg)

The purpose of this fork is to go forward w/o a need to wait for the upstream code reviews.
At the moment of writing this, I have 2 pending CLs to the upstream
([1](https://bitbucket.org/bitcraze/crazyflie-firmware/pull-request/9/fix-url-to-open-x-imu-algorithm/diff
),
[2](https://bitbucket.org/bitcraze/crazyflie-firmware/issue/14/incorrect-dt-passed-to-sensfusion6updateq
)).

The goals are:

* Adopt clang-format to enforce code formatting rules
  ([done](https://github.com/samofly/crazyflie-firmware/blob/krasin/scripts/clang-format-all.sh)
  for fork, pending for upstream)
* Setup Buildbot for Crazyflie firmware
  ([done](http://buildcop.org:29010/console) for fork, pending for upstream)
* Run clang static analyzer on Buildbot
* Add unit tests (run them inside QEMU)
* Generate firmware binaries for each green revision and host them on the external storage
* Run the firmware inside QEMU
* Run regression tests for IMU (based on to be recorded golden data)
* Run automated tests for the different radio protocols, that Crazyflie supports
* Rebase to a newer version of FreeRTOS
* Hard: Implement simulator environment with a physics model to test the quality of
  the copter stabilizer under different conditions
* Hard: expose REST API. See [REST.md](https://github.com/samofly/crazyflie-firmware/blob/krasin/REST.md)
  for more details.

#### [Buildbot console](http://buildcop.org:29010/console)


#### Directory structure:

```
./              | Root, contains the Makefile
 + init         | Contains the main.c
 + config       | Configuration files
 + drivers      | Hardware driver layer
 |  + src       | Drivers source code
 |  + interface | Drivers header files. Interface to the HAL layer
 + hal          | Hardware abstaction layer
 |  + src       | HAL source code
 |  + interface | HAL header files. Interface with the other parts of the program
 + modules      | Firmware operating code and headers
 |  + src       | Firmware tasks source code and main.c
 |  + interface | Operating headers. Configure the firmware environement
 + utils        | Utils code. Implement utility block like the console.
 |  + src       | Utils source code
 |  + interface | Utils header files. Interface with the other parts of the program
 + scripts      | Misc. scripts for LD, OpenOCD, make, version control, ...
 |              | *** The two following folders contains the unmodified files ***
 + lib          | Libraries
 |  + FreeRTOS  | Source FreeRTOS folder. Cleaned up from the useless files
 |  + STM32F... | Library folder of the St STM32 peripheral lib
 |  + CMSIS     | Core abstraction layer
```

#### Make targets:

```
all        : Shortcut for build
compile    : Compile cflie.hex. WARNING: Do NOT update version.c
build      : Update version.c and compile cflie.elf/hex
clean_o    : Clean only the Objects files, keep the executables (ie .elf, .hex)
clean      : Clean every compiled files
mrproper   : Clean every compiled files and the classical editors backup files
flash      : Flash cflie.elf using OpenOCD
halt       : Halt the target using OpenOCD
reset      : Reset the target using OpenOCD
openocd    : Launch OpenOCD
```
