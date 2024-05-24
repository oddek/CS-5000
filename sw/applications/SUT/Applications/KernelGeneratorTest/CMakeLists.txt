
set(APP_NAME KernelGeneratorTest)

file(GLOB SRC_FILES "*.cpp")

add_executable(${APP_NAME} ${SRC_FILES})

if(CMAKE_CROSSCOMPILING)
install(TARGETS ${APP_NAME} DESTINATION ${CMAKE_STAGING_PREFIX})
endif()

message(${CMAKE_SOURCE_DIR})

target_include_directories(${APP_NAME}
        PRIVATE
        "${LibNL_INCLUDE_DIR}"
        "${CMAKE_SOURCE_DIR}/../../drivers"
)

target_link_libraries(${APP_NAME}
        PUBLIC

        PRIVATE
        fmt
        Components
        ${LibNL_LIBRARIES}
    )