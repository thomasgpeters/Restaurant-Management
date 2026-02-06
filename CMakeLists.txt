cmake_minimum_required(VERSION 3.16)
project(RestaurantPOS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Find Wt libraries ──────────────────────────────────────────────────────
# Support both system-installed and locally-built Wt
find_library(WT_LIB wt HINTS /usr/local/lib /usr/lib)
find_library(WTHTTP_LIB wthttp HINTS /usr/local/lib /usr/lib)
find_library(WTDBO_LIB wtdbo HINTS /usr/local/lib /usr/lib)
find_library(WTDBO_SQLITE3_LIB wtdbosqlite3 HINTS /usr/local/lib /usr/lib)

find_path(WT_INCLUDE_DIR Wt/WApplication.h HINTS /usr/local/include /usr/include)

if(NOT WT_LIB OR NOT WTHTTP_LIB OR NOT WTDBO_LIB OR NOT WTDBO_SQLITE3_LIB)
    message(FATAL_ERROR "Could not find Wt libraries. "
        "Install Wt or set library paths with -DWT_LIB=...")
endif()

message(STATUS "Wt library:          ${WT_LIB}")
message(STATUS "WtHttp library:      ${WTHTTP_LIB}")
message(STATUS "WtDbo library:       ${WTDBO_LIB}")
message(STATUS "WtDbo SQLite3:       ${WTDBO_SQLITE3_LIB}")
message(STATUS "Wt includes:         ${WT_INCLUDE_DIR}")

# ─── SQLite3 ─────────────────────────────────────────────────────────────────
find_package(PkgConfig QUIET)
pkg_check_modules(SQLITE3 sqlite3)
if(NOT SQLITE3_FOUND)
    find_library(SQLITE3_LIBRARIES sqlite3)
    find_path(SQLITE3_INCLUDE_DIRS sqlite3.h)
endif()

# ─── Boost ───────────────────────────────────────────────────────────────────
find_package(Boost REQUIRED COMPONENTS filesystem thread)

# ─── Source files ────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/services/ApiService.cpp
    src/ui/RestaurantApp.cpp
    src/widgets/ManagerView.cpp
    src/widgets/FrontDeskView.cpp
    src/widgets/KitchenView.cpp
)

# ─── Executable ──────────────────────────────────────────────────────────────
add_executable(restaurant_pos ${SOURCES})

target_include_directories(restaurant_pos PRIVATE
    ${WT_INCLUDE_DIR}
    ${SQLITE3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(restaurant_pos PRIVATE
    ${WT_LIB}
    ${WTHTTP_LIB}
    ${WTDBO_LIB}
    ${WTDBO_SQLITE3_LIB}
    ${SQLITE3_LIBRARIES}
    ${Boost_LIBRARIES}
    pthread
)

# ─── WTHTTP configuration define ─────────────────────────────────────────────
target_compile_definitions(restaurant_pos PRIVATE
    WTHTTP_CONFIGURATION="${CMAKE_SOURCE_DIR}/wt_config.xml"
)

# ─── Copy resources to build directory ────────────────────────────────────────
add_custom_command(TARGET restaurant_pos POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_BINARY_DIR}/resources
    COMMENT "Copying resources to build directory"
)

# ─── Install ─────────────────────────────────────────────────────────────────
install(TARGETS restaurant_pos DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/restaurant_pos/resources)
