set(CMSIS_DIR ${CMAKE_SOURCE_DIR}/libs/cmsis)
set(DSP_LIB_DIR ${CMAKE_SOURCE_DIR}/libs/cmsis/dsp_lib)

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${DSP_LIB_DIR}")
  message(FATAL_ERROR "dsp_lib submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${CMSIS_DIR}")
  message(FATAL_ERROR "cmsis submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

include_directories(
    ${CMSIS_DIR}/core
    ${CMSIS_DIR}/device
)

file(GLOB DSP_LIB_SRC
    ${DSP_LIB_DIR}/BasicMathFunctions/*.c
    ${DSP_LIB_DIR}/CommonTables/*.c
    ${DSP_LIB_DIR}/ComplexMathFunctions/*.c
    ${DSP_LIB_DIR}/ControllerFunctions/*.c
    ${DSP_LIB_DIR}/FastMathFunctions/*.c
    ${DSP_LIB_DIR}/FilteringFunctions/*.c
    ${DSP_LIB_DIR}/MatrixFunctions/*.c
    ${DSP_LIB_DIR}/StatisticsFunctions/*.c
    ${DSP_LIB_DIR}/SupportFunctions/*.c
    ${DSP_LIB_DIR}/TransformFunctions/*.c
)

set(DSP_LIB_COMPILE_FLAGS "${STM32_DEFINES} -D__FPU_PRESENT=1")

set_source_files_properties(${DSP_LIB_SRC}
    PROPERTIES COMPILE_FLAGS ${DSP_LIB_COMPILE_FLAGS}
)

add_library(dsplib STATIC ${DSP_LIB_SRC})

set_target_properties(dsplib PROPERTIES LINKER_LANGUAGE C)

set(EXTERNAL_LIBS ${EXTERNAL_LIBS} dsplib)