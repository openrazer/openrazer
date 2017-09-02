cmake_minimum_required(VERSION 3.0.2)
project(openrazer)

set(SOURCES
        driver/razermouse_driver.c
        driver/razerfirefly_driver.c
        driver/razerkbd_driver.c
        driver/razercommon.c
        driver/razerchromacommon.c
        )

set(HEADERS
        driver/razermouse_driver.h
        driver/razerfirefly_driver.h
        driver/razerkbd_driver.h
        driver/razercommon.h
        driver/razerchromacommon.h
        )

set(TARGET_VARS
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        SOURCES ${HEADERS} ${SOURCES}
)

execute_process(COMMAND uname -r
        OUTPUT_VARIABLE INSTALLED_KERNEL
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(LINUX_KERNEL_PATH /usr/lib/modules/${INSTALLED_KERNEL})
include_directories(${LINUX_KERNEL_PATH}/build/include
    ${LINUX_KERNEL_PATH}/build/arch/x86/include
)

# Build all target
add_custom_target(All # all is reserved
        COMMAND make all
        ${TARGET_VARS}
)
add_custom_target(build-lp-all
        COMMAND make lp_all
        ${TARGET_VARS}
)

# Driver compilation
add_custom_target(driver
        COMMAND make driver
        ${TARGET_VARS}
)
add_custom_target(driver-verbose
        COMMAND make driver_verbose
        ${TARGET_VARS}
)
add_custom_target(driver-clean
        COMMAND make driver_clean
        ${TARGET_VARS}
)
# Install kernel modules and then update module dependencies
add_custom_target(driver-install
        COMMAND make driver_install
        ${TARGET_VARS}
)
# Remove kernel modules
add_custom_target(driver-uninstall
        COMMAND make driver_uninstall
        ${TARGET_VARS}
)

# Launchpad hacks
add_custom_target(lp-driver
        COMMAND make lp_driver
        ${TARGET_VARS}
)
add_custom_target(lp-driver-clean
        COMMAND make lp_driver_clean
        ${TARGET_VARS}
)

# Razer Daemon
add_custom_target(daemon-install
        COMMAND make daemon_install
        ${TARGET_VARS}
)
add_custom_target(daemon-uninstall
        COMMAND make daemon_uninstall
        ${TARGET_VARS}
)

# Python Library
add_custom_target(python-library-install
        COMMAND make python_library_install
        ${TARGET_VARS}
)
add_custom_target(python-library-uninstall
        COMMAND make python_library_uninstall
        ${TARGET_VARS}
)

# Clean target
add_custom_target(Clean # clean is reserved
        COMMAND make clean
        ${TARGET_VARS}
)

add_custom_target(setup-dkms
        COMMAND make setup_dkms
        ${TARGET_VARS}
)

add_custom_target(udev-install
        COMMAND make udev_install
        ${TARGET_VARS}
)
add_custom_target(udev-uninstall
        COMMAND make udev_uninstall
        ${TARGET_VARS}
)

# Install for Ubuntu
add_custom_target(ubuntu-install
        COMMAND make ubuntu_install
        ${TARGET_VARS}
)

add_custom_target(Install # install is reserved
        COMMAND make install
        ${TARGET_VARS}
)
add_custom_target(Uninstall # uninstall is reserved
        COMMAND make uninstall
        ${TARGET_VARS}
)

add_executable(FakeTarget ${HEADERS} ${SOURCES}) # fake target so kernel includes are found for autocompletion

target_compile_definitions(FakeTarget PUBLIC __KERNEL__=1)
