add_compile_definitions(_POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700)

if(APPLE)
    add_definitions(-D_DARWIN_C_SOURCE)
endif()

find_program(LINT "clang-tidy")
IF (LINT)
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*,-llvmlibc-restrict-system-libc-headers,-cppcoreguidelines-init-variables,-clang-analyzer-security.insecureAPI.strcpy,-concurrency-mt-unsafe,-android-cloexec-accept,-android-cloexec-dup,-google-readability-todo,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-cert-dcl03-c,-hicpp-static-assert,-misc-static-assert,-altera-struct-pack-align,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling;--quiet")
ENDIF ()

function(find_libxcrypt)
    if (TARGET COMP8005_project)
        return()
    endif ()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXCRYPT IMPORTED_TARGET libxcrypt)

    if (TARGET PkgConfig::LIBXCRYPT)
        add_library(COMP8005_project INTERFACE)
        target_link_libraries(COMP8005_project INTERFACE PkgConfig::LIBXCRYPT)
        target_compile_definitions(COMP8005_project INTERFACE HAVE_CRYPT_H)
    endif ()
endfunction()

function(find_sys_crypt)
    if (TARGET COMP8005_project)
        return()
    endif ()

    include(CheckIncludeFile)
    include(CheckSymbolExists)

    set(defs "")
    set(CMAKE_REQUIRED_INCLUDES "")
    set(CMAKE_REQUIRED_LIBRARIES "")

    check_include_file("crypt.h" HAVE_CRYPT_H)
    if (HAVE_CRYPT_H)
        list(APPEND defs HAVE_CRYPT_H)
        list(APPEND CMAKE_REQUIRED_INCLUDES "crypt.h")
    endif ()

    check_symbol_exists(crypt "unistd.h" HAVE_CRYPT)
    if (NOT HAVE_CRYPT)
        list(APPEND CMAKE_REQUIRED_LIBRARIES crypt)
        check_symbol_exists(crypt "unistd.h" HAVE_CRYPT_WITH_LIB)
    endif ()

    if (HAVE_CRYPT OR HAVE_CRYPT_WITH_LIB)
        add_library(COMP8005_project INTERFACE)
        target_link_libraries(COMP8005_project INTERFACE ${CMAKE_REQUIRED_LIBRARIES})
        target_compile_definitions(COMP8005_project INTERFACE ${defs})
    endif ()
endfunction()

option(PREFER_LIBXCRYPT "Prefer libxcrypt for crypt()" OFF)
if (PREFER_LIBXCRYPT)
    find_libxcrypt()
    find_sys_crypt()
else ()
    find_sys_crypt()
    find_libxcrypt()
endif ()

if (NOT TARGET COMP8005_project)
    message(FATAL_ERROR "Could not find a suitable crypt() implementation")
endif ()

find_package(OpenMP REQUIRED)

# Make an executable
add_executable(server ${COMMON_SOURCE_LIST} ${SERVER_SOURCE_LIST} ${HEADER_LIST})
add_executable(client ${COMMON_SOURCE_LIST} ${CLIENT_SOURCE_LIST} ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(server PRIVATE ../include)
target_include_directories(server PRIVATE /usr/include)
target_include_directories(server PRIVATE /usr/local/include)
target_link_directories(server PRIVATE /usr/lib)
target_link_directories(server PRIVATE /usr/local/lib)
target_include_directories(client PRIVATE ../include)
target_include_directories(client PRIVATE /usr/include)
target_include_directories(client PRIVATE /usr/local/include)
target_link_directories(client PRIVATE /usr/lib)
target_link_directories(client PRIVATE /usr/local/lib)

# All users of this library will need at least C17
target_compile_features(server PUBLIC c_std_17)
target_compile_options(server PRIVATE -g)
target_compile_options(server PRIVATE -fstack-protector-all -ftrapv)
target_compile_options(server PRIVATE -Wpedantic -Wall -Wextra)
target_compile_options(server PRIVATE -Wdouble-promotion -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wnull-dereference -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-local-typedefs -Wstrict-overflow=5 -Wmissing-noreturn -Walloca -Wfloat-equal -Wdeclaration-after-statement -Wshadow -Wpointer-arith -Wabsolute-value -Wundef -Wexpansion-to-defined -Wunused-macros -Wno-endif-labels -Wbad-function-cast -Wcast-qual -Wwrite-strings -Wconversion -Wdangling-else -Wdate-time -Wempty-body -Wsign-conversion -Wfloat-conversion -Waggregate-return -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wpacked -Wredundant-decls -Wnested-externs -Winline -Winvalid-pch -Wlong-long -Wvariadic-macros -Wdisabled-optimization -Wstack-protector -Woverlength-strings)
target_compile_features(client PUBLIC c_std_17)
target_compile_options(client PRIVATE -g)
target_compile_options(client PRIVATE -fstack-protector-all -ftrapv)
target_compile_options(client PRIVATE -Wpedantic -Wall -Wextra)
target_compile_options(client PRIVATE -Wdouble-promotion -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wnull-dereference -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-local-typedefs -Wstrict-overflow=5 -Wmissing-noreturn -Walloca -Wfloat-equal -Wdeclaration-after-statement -Wshadow -Wpointer-arith -Wabsolute-value -Wundef -Wexpansion-to-defined -Wunused-macros -Wno-endif-labels -Wbad-function-cast -Wcast-qual -Wwrite-strings -Wconversion -Wdangling-else -Wdate-time -Wempty-body -Wsign-conversion -Wfloat-conversion -Waggregate-return -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wpacked -Wredundant-decls -Wnested-externs -Winline -Winvalid-pch -Wlong-long -Wvariadic-macros -Wdisabled-optimization -Wstack-protector -Woverlength-strings)

find_library(LIBM m REQUIRED)
find_library(LIBSOCKET socket)

target_link_libraries(server PRIVATE ${LIBM} OpenMP::OpenMP_C)
target_link_libraries(client PRIVATE ${LIBM} OpenMP::OpenMP_C)
target_link_libraries(server PRIVATE COMP8005_project)
target_link_libraries(client PRIVATE COMP8005_project)

set_target_properties(server PROPERTIES OUTPUT_NAME "server")
set_target_properties(client PROPERTIES OUTPUT_NAME "client")

#install(TARGETS server client DESTINATION bin)

# IDEs should put the headers in a nice place
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)

add_custom_target(
        format
        COMMAND clang-format
        -i
        ${HEADER_LIST}
        ${COMMON_SOURCE_LIST}
        ${SERVER_SOURCE_LIST}
        ${CLIENT_SOURCE_LIST}
)