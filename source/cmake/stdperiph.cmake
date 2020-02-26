set(STDPERIPH_DIR ${CMAKE_SOURCE_DIR}/libs/STM32F30x_StdPeriph_Driver)
set(CMSIS_DIR ${CMAKE_SOURCE_DIR}/libs/cmsis)
set(LINKER_SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/config/LinkerScripts)

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${STDPERIPH_DIR}")
  message(FATAL_ERROR "stdperiph submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${CMSIS_DIR}")
  message(FATAL_ERROR "cmsis submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

include_directories(
    ${CMSIS_DIR}/core
    ${CMSIS_DIR}/device
    ${I2C_CPAL_DIR}/inc
    ${STDPERIPH_DIR}/inc
)

set(STDPERIPH_LIB_SRC
    ${CMSIS_DIR}/device/system_stm32f30x.c
    ${STDPERIPH_DIR}/src/stm32f30x_adc.c
    ${STDPERIPH_DIR}/src/stm32f30x_can.c
    ${STDPERIPH_DIR}/src/stm32f30x_comp.c
    ${STDPERIPH_DIR}/src/stm32f30x_crc.c
    ${STDPERIPH_DIR}/src/stm32f30x_dac.c
    ${STDPERIPH_DIR}/src/stm32f30x_dbgmcu.c
    ${STDPERIPH_DIR}/src/stm32f30x_dma.c
    ${STDPERIPH_DIR}/src/stm32f30x_exti.c
    ${STDPERIPH_DIR}/src/stm32f30x_flash.c
    ${STDPERIPH_DIR}/src/stm32f30x_fmc.c
    ${STDPERIPH_DIR}/src/stm32f30x_gpio.c
    ${STDPERIPH_DIR}/src/stm32f30x_hrtim.c
    ${STDPERIPH_DIR}/src/stm32f30x_i2c.c
    ${STDPERIPH_DIR}/src/stm32f30x_iwdg.c
    ${STDPERIPH_DIR}/src/stm32f30x_misc.c
    ${STDPERIPH_DIR}/src/stm32f30x_opamp.c
    ${STDPERIPH_DIR}/src/stm32f30x_pwr.c
    ${STDPERIPH_DIR}/src/stm32f30x_rcc.c
    ${STDPERIPH_DIR}/src/stm32f30x_rtc.c
    ${STDPERIPH_DIR}/src/stm32f30x_spi.c
    ${STDPERIPH_DIR}/src/stm32f30x_syscfg.c
    ${STDPERIPH_DIR}/src/stm32f30x_tim.c
    ${STDPERIPH_DIR}/src/stm32f30x_usart.c
    ${STDPERIPH_DIR}/src/stm32f30x_wwdg.c
)

set(STM32_DEFINES "${STM32_DEFINES} -DUSE_STDPERIPH_DRIVER")

set_source_files_properties(${STDPERIPH_LIB_SRC}
    PROPERTIES COMPILE_FLAGS ${STM32_DEFINES}
)

add_library(stdperiph STATIC ${STDPERIPH_LIB_SRC})

set_target_properties(stdperiph PROPERTIES LINKER_LANGUAGE C)

# add startup and linker file
set(STARTUP_ASM_FILE "${CMSIS_DIR}/device/startup_stm32f30x.s")
set_property(SOURCE ${STARTUP_ASM_FILE} PROPERTY LANGUAGE ASM)
set(LINKER_FILE "${LINKER_SCRIPTS_DIR}/STM32F303xC/STM32F303VC_FLASH.ld")


set(EXTERNAL_EXECUTABLES ${EXTERNAL_EXECUTABLES} ${STARTUP_ASM_FILE})

set(EXTERNAL_LIBS ${EXTERNAL_LIBS} stdperiph)