cmake_minimum_required (VERSION 2.8)
project(crazyflie-firmware C ASM)

include(ExternalProject)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_C_COMPILER arm-none-eabi-gcc)
SET(CMAKE_CXX_COMPILER arm-none-eabi-g++)
SET(CMAKE_ASM_COMPILER arm-none-eabi-as)

SET(CMAKE_C_FLAGS "-Os -g3 -std=c99 -mcpu=cortex-m3 -mthumb -L ${CMAKE_SOURCE_DIR}/scripts -T ${CMAKE_SOURCE_DIR}/scripts/STM32F103_32K_20K_FLASH_CLOAD.ld -DSTM32F10X_MD -include stm32f10x_conf.h -ffunction-sections -fdata-sections -Wl,--cref,--gc-sections -DBOARD_REV_E -Wall -fno-strict-aliasing")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

include_directories(
  config
  lib/FreeRTOS/include
  lib/FreeRTOS/portable/GCC/ARM_CM3
  drivers/interface
  modules/interface
  lib/STM32F10x_StdPeriph_Driver/inc
  lib/CMSIS/Core/CM3
  utils/interface
  hal/interface
)

add_executable(cflie.elf
  init/main.c
  lib/CMSIS/Core/CM3/startup/gcc/startup_stm32f10x_md.s
  drivers/src/led.c
  drivers/src/motors.c
  drivers/src/nrf24l01.c
  drivers/src/hmc5883l.c
  drivers/src/i2croutines.c
  drivers/src/ms5611.c
  drivers/src/exti.c
  drivers/src/nvic.c
  drivers/src/i2cdev.c
  drivers/src/mpu6050.c
  drivers/src/adc.c
  utils/src/configblock.c
  utils/src/debug.c
  utils/src/cfassert.c
  utils/src/eprintf.c
  utils/src/cpuid.c
  utils/src/crc.c
  utils/src/fp16.c
  utils/src/filter.c
  hal/src/pm.c
  hal/src/radiolink.c
  hal/src/uart.c
  hal/src/imu.c
  hal/src/eskylink.c
  hal/src/ledseq.c
  hal/src/freeRTOSdebug.c
  modules/src/log.c
  modules/src/crtpservice.c
  modules/src/pidctrl.c
  modules/src/sensfusion6.c
  modules/src/worker.c
  modules/src/stabilizer.c
  modules/src/commander.c
  modules/src/comm.c
  modules/src/pid.c
  modules/src/controller.c
  modules/src/crtp.c
  modules/src/param.c
  modules/src/system.c
  modules/src/console.c
  lib/FreeRTOS/list.c
  lib/FreeRTOS/portable/GCC/ARM_CM3/port.c
  lib/FreeRTOS/portable/MemMang/heap_4.c
  lib/FreeRTOS/timers.c
  lib/FreeRTOS/tasks.c
  lib/FreeRTOS/queue.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.c
  lib/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
  lib/STM32F10x_StdPeriph_Driver/src/misc.c
)

target_link_libraries(cflie.elf m)

add_custom_command(OUTPUT cflie.tmp
                   COMMAND arm-none-eabi-objcopy cflie.elf -O binary --pad-to 0 cflie.tmp
                   COMMAND ${CMAKE_COMMAND} -E copy cflie.tmp cflie.bin
                   DEPENDS cflie.elf
                   COMMENT "objcopy cflie.elf to cflie.bin")
add_custom_target(cflie.bin ALL DEPENDS cflie.tmp)

add_executable(hello.elf
  test/hello.c)

enable_testing()

add_test(hello_test qemu-system-arm -M stm32-p103 -nographic
  -monitor null -serial null -semihosting -kernel hello.elf)
