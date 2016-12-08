cmake_minimum_required (VERSION 2.6)
project (bcrypt)

# build
include_directories(
    ${CMAKE_SOURCE_DIR}/bcrypt
)

file(GLOB bcrypt_src
    bcrypt/*.c
    bcrypt/crypt_blowfish/*.c
)
add_library(bcrypt STATIC ${bcrypt_src})

add_executable(bcrypt_test bcrypt/bcrypt.c)
target_compile_options(bcrypt_test PUBLIC "-DTEST_BCRYPT")
target_link_libraries(bcrypt_test bcrypt)

enable_testing()
add_test(bcrypt_test bcrypt_test)

# install
install(DIRECTORY bcrypt DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS bcrypt EXPORT DESTINATION lib)