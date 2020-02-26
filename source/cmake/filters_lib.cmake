set(FILTERS_LIB_DIR ${CMAKE_SOURCE_DIR}/libs/filters_lib)

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${FILTERS_LIB_DIR}")
  message(FATAL_ERROR "dsp_lib submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${CMSIS_DIR}")
  message(FATAL_ERROR "cmsis submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

include_directories(
    ${CMSIS_DIR}/core
    ${CMSIS_DIR}/device
    ${FILTERS_LIB_DIR}/inc
)

file(GLOB FILTERS_LIB_SRC
    ${FILTERS_LIB_DIR}/src/*.c
)

set(FILTERS_LIB_COMPILE_FLAGS "${STM32_DEFINES}")

set_source_files_properties(${FILTERS_LIB_SRC}
    PROPERTIES COMPILE_FLAGS ${FILTERS_LIB_COMPILE_FLAGS}
)

add_library(filterslib STATIC ${FILTERS_LIB_SRC})

set_target_properties(filterslib PROPERTIES LINKER_LANGUAGE C)

set(EXTERNAL_LIBS ${EXTERNAL_LIBS} filterslib)

target_link_libraries(filterslib dsplib)