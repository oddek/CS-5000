
set(APP_NAME TestTest)

file(GLOB SRC_FILES "*.cpp")

add_executable(${APP_NAME} ${SRC_FILES})

if(CMAKE_CROSSCOMPILING)
    install(TARGETS ${APP_NAME} DESTINATION ${CMAKE_STAGING_PREFIX})
endif()

target_link_libraries(${APP_NAME}
        PUBLIC

        PRIVATE
        fmt
        Components
)
