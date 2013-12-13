FIND_PROGRAM( GCOV_PATH gcov )
FIND_PROGRAM( LCOV_PATH lcov )
FIND_PROGRAM( GENHTML_PATH genhtml )
FIND_PROGRAM( GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/tests)

IF(NOT GCOV_PATH)
    MESSAGE(FATAL_ERROR "gcov not found! Aborting...")
ENDIF() # NOT GCOV_PATH

IF(NOT LCOV_PATH)
    MESSAGE(FATAL_ERROR "lcov not found! Aborting...")
ENDIF() # NOT LCOV_PATH

IF(NOT GENHTML_PATH)
    MESSAGE(FATAL_ERROR "genhtml not found! Aborting...")
ENDIF() # NOT GENHTM

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
    # Clang version 3.0.0 and greater now supports gcov as well.
    MESSAGE(WARNING "Compiler is not GNU gcc! Clang Version 3.0.0 and greater supports gcov as well, but older versions don't.")

    IF(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        MESSAGE(FATAL_ERROR "Compiler is not GNU gcc! Aborting...")
    ENDIF()
ENDIF() # NOT CMAKE_COMPILER_IS_GNUCXX

IF ( NOT CMAKE_BUILD_TYPE STREQUAL "Debug" )
  MESSAGE( WARNING "Code coverage results with an optimized (non-Debug) build may be misleading" )
ENDIF() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"

ADD_CUSTOM_TARGET(coverage
  COMMAND make test # Workaround for add_dependencies because test is not a real target
  COMMAND mkdir -p coverage
  COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage/coverage.info
  COMMAND ${LCOV_PATH} --remove coverage/coverage.info 'tests/*' '/usr/*' --output-file coverage/coverage.info.cleaned
  COMMAND ${GENHTML_PATH} -o coverage/ coverage/coverage.info.cleaned
  COMMAND ${CMAKE_COMMAND} -E remove coverage/coverage.info coverage/coverage.info.cleaned

  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
