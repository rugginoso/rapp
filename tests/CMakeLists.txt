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

    link_directories(${rapp_BINARY_DIR}/src)
    link_directories(${rapp_BINARY_DIR}/tests)

    add_library(rapp_test STATIC test_utils.c test_dlstubs.c test_memstubs.c)

    set(LIBCHECK_DEPS m rt pthread)
    set(TEST_LIBS rapp_core rapp_test ${LIBCHECK_LIBRARY} ${LIBCHECK_DEPS})

    add_executable(check_stub check_stub.c)
    target_link_libraries(check_stub ${LIBCHECK_LIBRARY} ${LIBCHECK_DEPS})

    # collector
    add_executable(check_collector check_collector.c)
    target_link_libraries(check_collector ${TEST_LIBS})
    add_test(test_collector ${EXECUTABLE_OUTPUT_PATH}/check_collector)

    # container
    add_executable(check_container check_container.c)
    target_link_libraries(check_container ${TEST_LIBS})
    add_test(test_container ${EXECUTABLE_OUTPUT_PATH}/check_container)

    # event loop
    add_executable(check_eloop check_eloop.c)
    target_link_libraries(check_eloop ${TEST_LIBS})
    add_test(test_eloop ${EXECUTABLE_OUTPUT_PATH}/check_eloop)

    # logger
    add_executable(check_logger check_logger.c)
    target_link_libraries(check_logger ${TEST_LIBS})
    add_test(test_logger ${EXECUTABLE_OUTPUT_PATH}/check_logger)

    # signal handler
    add_executable(check_signalhandler check_signalhandler.c)
    target_link_libraries(check_signalhandler ${TEST_LIBS})
    add_test(test_signalhandler ${EXECUTABLE_OUTPUT_PATH}/check_signalhandler)

    # tcp server
    add_executable(check_tcpserver check_tcpserver.c)
    target_link_libraries(check_tcpserver ${TEST_LIBS})
    add_test(test_tcpserver ${EXECUTABLE_OUTPUT_PATH}/check_tcpserver)

    # tcp connection
    add_executable(check_tcpconnection check_tcpconnection.c)
    target_link_libraries(check_tcpconnection ${TEST_LIBS})
    add_test(test_tcpconnection ${EXECUTABLE_OUTPUT_PATH}/check_tcpconnection)

    add_executable(check_config_yaml_parser check_config_yaml.c)
    target_link_libraries(check_config_yaml_parser ${TEST_LIBS} ${LIBYAML_LIBRARIES})
    add_test(test_config_yaml ${EXECUTABLE_OUTPUT_PATH}/check_config_yaml_parser)

    add_executable(check_config check_config.c)
    target_link_libraries(check_config ${TEST_LIBS})
    add_test(test_config ${EXECUTABLE_OUTPUT_PATH}/check_config)

    add_executable(check_config_commandline check_config_commandline.c)
    target_link_libraries(check_config_commandline ${TEST_LIBS} ${LIBYAML_LIBRARIES})
    add_test(test_config_commandline ${EXECUTABLE_OUTPUT_PATH}/check_config_commandline)

    add_executable(check_config_env check_config_env.c)
    target_link_libraries(check_config_env ${TEST_LIBS} ${LIBYAML_LIBRARIES})
    add_test(test_config_env ${EXECUTABLE_OUTPUT_PATH}/check_config_env)

    add_executable(check_httprequestqueue check_httprequestqueue.c)
    target_link_libraries(check_httprequestqueue ${TEST_LIBS})
    add_test(test_httprequestqueue ${EXECUTABLE_OUTPUT_PATH}/check_httprequestqueue)

    add_executable(check_httprequest check_httprequest.c)
    target_link_libraries(check_httprequest ${TEST_LIBS})
    add_test(test_httprequest ${EXECUTABLE_OUTPUT_PATH}/check_httprequest)

    add_executable(check_httpresponse check_httpresponse.c)
    target_link_libraries(check_httpresponse ${TEST_LIBS})
    add_test(test_httpresponse ${EXECUTABLE_OUTPUT_PATH}/check_httpresponse)

    add_executable(check_httprouter check_httprouter.c)
    target_link_libraries(check_httprouter ${TEST_LIBS})
    add_test(test_httprouter ${EXECUTABLE_OUTPUT_PATH}/check_httprouter)

    add_executable(check_version check_version.c)
    target_link_libraries(check_version ${TEST_LIBS})
    add_test(test_version ${EXECUTABLE_OUTPUT_PATH}/check_version)

endif(ENABLE_TESTS)

