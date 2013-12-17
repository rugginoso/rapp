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
    include_directories(${rapp_SOURCE_DIR}/contrib/http-parser)
    # for the custom headers - if any
    include_directories(${rapp_SOURCE_DIR}/tests)

    if(CMAKE_COMPILER_IS_GNUCC)
        option(ENABLE_TESTS_COVERAGE "build test coverage report")
        if (ENABLE_TESTS_COVERAGE)
            message(STATUS "Enabling the code coverage support")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
            include(TestCoverage)
        endif(ENABLE_TESTS_COVERAGE)
    endif(CMAKE_COMPILER_IS_GNUCC)

    add_executable(check_stub check_stub.c)
    target_link_libraries(check_stub ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_stub ${EXECUTABLE_OUTPUT_PATH}/check_stub)

    # collector
    add_executable(check_collector
        check_collector.c
        ${rapp_SOURCE_DIR}/src/collector.c
    )
    target_link_libraries(check_collector ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_collector ${EXECUTABLE_OUTPUT_PATH}/check_collector)

    # container
    add_executable(check_container
        check_container.c
        ${rapp_SOURCE_DIR}/src/container.c
        ${rapp_SOURCE_DIR}/src/logger.c
        ${rapp_SOURCE_DIR}/tests/test_dlstubs.c
    )
    target_link_libraries(check_container ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_container ${EXECUTABLE_OUTPUT_PATH}/check_container)

    # event loop
    add_executable(check_eloop
        check_eloop.c
        ${rapp_SOURCE_DIR}/src/eloop.c
        ${rapp_SOURCE_DIR}/src/collector.c
    )
    target_link_libraries(check_eloop ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_eloop ${EXECUTABLE_OUTPUT_PATH}/check_eloop)

    # logger
    add_executable(check_logger
        check_logger.c
        ${rapp_SOURCE_DIR}/src/logger.c
    )
    target_link_libraries(check_logger ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_logger ${EXECUTABLE_OUTPUT_PATH}/check_logger)

    # signal handler
    add_executable(check_signalhandler
        check_signalhandler.c
        ${rapp_SOURCE_DIR}/src/signalhandler.c
        ${rapp_SOURCE_DIR}/src/eloop.c
        ${rapp_SOURCE_DIR}/src/collector.c
    )
    target_link_libraries(check_signalhandler ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_signalhandler ${EXECUTABLE_OUTPUT_PATH}/check_signalhandler)

    # tcp server
    add_executable(check_tcpserver
        check_tcpserver.c
        ${rapp_SOURCE_DIR}/tests/test_utils.c
        ${rapp_SOURCE_DIR}/src/tcpserver.c
        ${rapp_SOURCE_DIR}/src/tcpconnection.c
        ${rapp_SOURCE_DIR}/src/eloop.c
        ${rapp_SOURCE_DIR}/src/collector.c
    )
    target_link_libraries(check_tcpserver ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_tcpserver ${EXECUTABLE_OUTPUT_PATH}/check_tcpserver)

    # tcp connection
    add_executable(check_tcpconnection
        check_tcpconnection.c
        ${rapp_SOURCE_DIR}/tests/test_utils.c
        ${rapp_SOURCE_DIR}/src/tcpconnection.c
        ${rapp_SOURCE_DIR}/src/eloop.c
        ${rapp_SOURCE_DIR}/src/collector.c
    )
    target_link_libraries(check_tcpconnection ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_tcpconnection ${EXECUTABLE_OUTPUT_PATH}/check_tcpconnection)

    add_executable(check_httprequest
        check_httprequest.c
        ${rapp_SOURCE_DIR}/src/httprequest.c
        ${rapp_SOURCE_DIR}/contrib/http-parser/http_parser.c
    )
    target_link_libraries(check_httprequest ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_httprequest ${EXECUTABLE_OUTPUT_PATH}/check_httprequest)

    add_executable(check_httpresponsewriter
        check_httpresponsewriter.c
        ${rapp_SOURCE_DIR}/tests/test_utils.c
        ${rapp_SOURCE_DIR}/src/httpresponsewriter.c
        ${rapp_SOURCE_DIR}/src/tcpconnection.c
        ${rapp_SOURCE_DIR}/src/eloop.c
        ${rapp_SOURCE_DIR}/src/collector.c
    )
    target_link_libraries(check_httpresponsewriter ${LIBCHECK_LIBRARY} m rt pthread)
    add_test(test_httpresponsewriter ${EXECUTABLE_OUTPUT_PATH}/check_httpresponsewriter)

endif(ENABLE_TESTS)

