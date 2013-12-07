option(ENABLE_TESTS "compile testsuite")

if(ENABLE_TESTS)
    find_package(LIBCHECK)
    if(LIBCHECK_NOTFOUND)
        message(FATAL_ERROR, "libcheck not found (get it at http://check.sf.net) - can't compile testsuite")
    else(LIBCHECK_NOTFOUND)
        message(STATUS "Enabling the test suite")
    endif(LIBCHECK_NOTFOUND)

    include_directories(${LIBCHECK_INCLUDE_DIR})

    include_directories(${rapp_BINARY_DIR})
    include_directories(${rapp_SOURCE_DIR})
    include_directories(${rapp_SOURCE_DIR}/src)
    # for the custom headers - if any
    include_directories(${rapp_BINARY_DIR}/tests)

    if(CMAKE_COMPILER_IS_GNUCC)
        message(STATUS "Enabling the code coverage support")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
	set(EON3D_DBG_LIBS ${EON3D_DBG_LIBS} gcov)
    endif(CMAKE_COMPILER_IS_GNUCC)

    add_executable(check_stub check_stub.c)
    add_test(test_stub ${EXECUTABLE_OUTPUT_PATH}/check_stub)
endif(ENABLE_TESTS)

