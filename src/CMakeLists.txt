include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(scheduler
  Exception.h
  ExecutionController.cpp
  ExecutionController.h
  ServiceController.cpp
  ServiceController.h
  Utility.cpp
  Utility.h
  )

set(TARGET scheduler_cli)
add_executable(${TARGET} main.cpp)
target_link_libraries(${TARGET} scheduler)
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(TARGET scheduler_service)
add_definitions(-DHTTP_LISTENER_PORT=${HTTP_LISTENER_PORT})
add_executable(${TARGET} service.cpp)
target_link_libraries(${TARGET} scheduler ${Boost_SYSTEM_LIBRARY} cpprestsdk::cpprest)
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
