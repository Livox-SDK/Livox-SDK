include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Name of the package to use when calling find_package(<package>) in consumer code. We use
# kebap-case because vcpkg uses that for its port names.
set(package livox-sdk)

set(livox_sdk_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/${package}")
set(livox_sdk_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/${package}")
# Set the install directory for the *config.cmake files to the path where vcpkg looks by default
set(livox_sdk_INSTALL_CMAKEDIR "share/${package}")

# Install target types to the given destinations and create an export.
install(TARGETS ${SDK_LIBRARY} EXPORT livox-sdkTargets
    ARCHIVE DESTINATION "${livox_sdk_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${livox_sdk_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${livox_sdk_INSTALL_INCLUDEDIR}")

# Copy the package config file to the install location and give it the correct name
install(FILES ../cmake/install-config.cmake
    DESTINATION ${livox_sdk_INSTALL_CMAKEDIR}
    RENAME "${package}Config.cmake")

write_basic_package_version_file("../${package}ConfigVersion.cmake"
    VERSION ${LIVOX_SDK_VERSION_STRING}
    COMPATIBILITY SameMajorVersion)

# Copy the package version file the the install location
install(FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION ${livox_sdk_INSTALL_CMAKEDIR})

# Generate and install a CMake file with code for installing targets from the given export
install(EXPORT livox-sdkTargets
    NAMESPACE livox_sdk::
    DESTINATION ${livox_sdk_INSTALL_CMAKEDIR})
