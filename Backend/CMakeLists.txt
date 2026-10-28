cmake_minimum_required(VERSION 4.2)

project(ProjectJHBackend VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "ProjectJH backend requires Windows and MSVC.")
endif()

add_library(jh_backend_options INTERFACE)
# 빌드 설정은 C++17로 유지하고 직접 작성하는 코드는 C++11 범위로 제한한다.
target_compile_features(jh_backend_options INTERFACE cxx_std_17)
set(CMAKE_CXX_EXTENSIONS OFF)
target_compile_definitions(jh_backend_options INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)
# Compile both source text and narrow string literals as UTF-8.
target_compile_options(jh_backend_options INTERFACE /W4 /permissive- /utf-8)

add_library(jh_backend_core STATIC
    Core/Types.h
    Core/Container.h
    Core/IocpCore.cpp
    Core/IocpCore.h
    Core/IocpEvent.cpp
    Core/IocpEvent.h
    Core/NetAddress.cpp
    Core/NetAddress.h
    Core/SocketUtils.cpp
    Core/SocketUtils.h
    Core/StatusCode.cpp
    Core/StatusCode.h
    Core/CoreGlobal.cpp
    Core/CoreGlobal.h
    Core/Logger.cpp
    Core/Logger.h
    Core/WinsockRuntime.cpp
    Core/WinsockRuntime.h
)
target_include_directories(jh_backend_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(jh_backend_core PUBLIC jh_backend_options PRIVATE ws2_32 shell32)

add_executable(jh_backend_server Server/Main.cpp)
target_link_libraries(jh_backend_server PRIVATE jh_backend_core)

add_executable(jh_backend_test_client TestClient/Main.cpp)
target_link_libraries(jh_backend_test_client PRIVATE jh_backend_core)

foreach(backend_target IN ITEMS jh_backend_core jh_backend_server jh_backend_test_client)
    target_precompile_headers(${backend_target} PRIVATE Core/Pch.h)
endforeach()

include(CTest)
if(BUILD_TESTING)
    # 로컬 테스트 소스는 Git에 포함하지 않는다.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/IocpCoreTests.cpp")
        add_executable(jh_backend_iocp_tests Tests/IocpCoreTests.cpp)
        target_link_libraries(jh_backend_iocp_tests PRIVATE jh_backend_core)
        add_test(NAME backend_iocp_core COMMAND jh_backend_iocp_tests)
        set_tests_properties(backend_iocp_core PROPERTIES TIMEOUT 15)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/NetworkTests.cpp")
        add_executable(jh_backend_network_tests Tests/NetworkTests.cpp)
        target_link_libraries(jh_backend_network_tests PRIVATE jh_backend_core)
        add_test(NAME backend_network_basics COMMAND jh_backend_network_tests)
        set_tests_properties(backend_network_basics PROPERTIES TIMEOUT 15)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/LoggerTests.cpp"
        AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/LoggerCaller.cpp")
        add_executable(jh_backend_logger_tests Tests/LoggerTests.cpp Tests/LoggerCaller.cpp)
        target_link_libraries(jh_backend_logger_tests PRIVATE jh_backend_core)
        add_test(NAME backend_logger COMMAND jh_backend_logger_tests)
        set_tests_properties(backend_logger PROPERTIES TIMEOUT 30)
    endif()
    add_test(NAME backend_server_startup COMMAND jh_backend_server)
    add_test(NAME backend_test_client_startup COMMAND jh_backend_test_client)
    set_tests_properties(backend_server_startup backend_test_client_startup
        PROPERTIES TIMEOUT 10
    )
endif()
