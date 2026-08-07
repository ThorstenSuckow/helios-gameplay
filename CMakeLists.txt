cmake_minimum_required(VERSION 3.28)

project(
        helios-gameplay
        VERSION 0.1.0
        LANGUAGES C CXX
)

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

include(CTest)
include(FetchContent)

# ------------------------------------------------------------------------------
# Dependencies (local sibling checkout, else fetched from git)
# ------------------------------------------------------------------------------

set(
        HELIOS_ENGINE_LOCAL_PATH
        "${CMAKE_CURRENT_LIST_DIR}/../helios-engine"
        CACHE PATH
        "Local path to helios-engine repository"
)

set(
        HELIOS_PHYSICS_LOCAL_PATH
        "${CMAKE_CURRENT_LIST_DIR}/../helios-physics"
        CACHE PATH
        "Local path to helios-physics repository"
)

if(NOT TARGET helios::engine)
    if(EXISTS "${HELIOS_ENGINE_LOCAL_PATH}/CMakeLists.txt")
        add_subdirectory(
                "${HELIOS_ENGINE_LOCAL_PATH}"
                "${CMAKE_BINARY_DIR}/_deps/helios-engine-build"
        )
    else()
        FetchContent_Declare(
                helios_engine
                GIT_REPOSITORY https://github.com/thorstensuckow/helios-engine.git
                GIT_TAG main
        )
        FetchContent_MakeAvailable(helios_engine)
    endif()
endif()

if(NOT TARGET helios::physics)
    if(EXISTS "${HELIOS_PHYSICS_LOCAL_PATH}/CMakeLists.txt")
        add_subdirectory(
                "${HELIOS_PHYSICS_LOCAL_PATH}"
                "${CMAKE_BINARY_DIR}/_deps/helios-physics-build"
        )
    else()
        FetchContent_Declare(
                helios_physics
                GIT_REPOSITORY https://github.com/thorstensuckow/helios-physics.git
                GIT_TAG main
        )
        FetchContent_MakeAvailable(helios_physics)
    endif()
endif()

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

file(
        GLOB_RECURSE CPP_FILES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(
        GLOB_RECURSE MODULE_INTERFACE_FILES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx"
)

list(
        FILTER CPP_FILES
        EXCLUDE REGEX
        "/CMakeFiles/|/CMakeTmp/|/cmake_build_|/build/"
)

list(
        FILTER MODULE_INTERFACE_FILES
        EXCLUDE REGEX
        "/CMakeFiles/|/CMakeTmp/|/cmake_build_|/build/"
)

# ------------------------------------------------------------------------------
# Library
# ------------------------------------------------------------------------------

add_library(helios_gameplay STATIC)
add_library(helios::gameplay ALIAS helios_gameplay)

set_property(
        TARGET helios_gameplay
        PROPERTY CXX_SCAN_FOR_MODULES ON
)

target_compile_features(
        helios_gameplay
        PUBLIC
        cxx_std_23
)

if(CPP_FILES)
    target_sources(
            helios_gameplay
            PRIVATE
            ${CPP_FILES}
    )
endif()

if(MODULE_INTERFACE_FILES)
    target_sources(
            helios_gameplay
            PUBLIC
            FILE_SET modules
            TYPE CXX_MODULES
            BASE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/src"
            FILES
            ${MODULE_INTERFACE_FILES}
    )
endif()

target_link_libraries(
        helios_gameplay
        PUBLIC
        helios::engine
        helios::physics
)

target_include_directories(
        helios_gameplay
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if(PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)

    set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
    set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)

    file(
            GLOB_RECURSE TEST_SOURCES
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.test.cpp"
    )

    if(TEST_SOURCES)
        add_executable(
                helios-gameplay-tests
                ${TEST_SOURCES}
        )

        target_link_libraries(
                helios-gameplay-tests
                PRIVATE
                helios::gameplay
                GTest::gtest_main
        )

        include(GoogleTest)
        gtest_discover_tests(helios-gameplay-tests)
    endif()

endif()