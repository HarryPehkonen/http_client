# Find Deno executable
find_program(DENO_EXECUTABLE deno REQUIRED)
message(STATUS "Found Deno: ${DENO_EXECUTABLE}")

# Function to manage test server
function(add_test_server TARGET_NAME SERVER_PORT)
    set(SERVER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/server/test_server.ts")
    set(PID_FILE "${CMAKE_BINARY_DIR}/test_server.pid")
    
    # Start server script
    add_custom_target(start_test_server
        COMMAND ${CMAKE_COMMAND} -E echo "Starting test server on port ${SERVER_PORT}..."
        COMMAND ${DENO_EXECUTABLE} run --allow-net ${SERVER_SCRIPT} & echo $$! > ${PID_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/server
    )
    
    # Stop server script
    add_custom_target(stop_test_server
        COMMAND ${CMAKE_COMMAND} -E echo "Stopping test server..."
        COMMAND pkill -F ${PID_FILE} || true
        COMMAND ${CMAKE_COMMAND} -E remove ${PID_FILE}
    )
    
    # Make the test target depend on server management
    add_dependencies(${TARGET_NAME} start_test_server)
    
    # Set server port for tests
    target_compile_definitions(${TARGET_NAME}
        PRIVATE
        TEST_SERVER_PORT=${SERVER_PORT}
    )
endfunction()

# Function to set test cleanup after test is defined
function(configure_test_cleanup TEST_NAME)
    set_property(TEST ${TEST_NAME}
        PROPERTY FIXTURES_CLEANUP stop_test_server
    )
endfunction()
