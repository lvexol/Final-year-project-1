include_directories(${CMAKE_SOURCE_DIR}/common)

add_executable(vexovncpasswd 
  kasmvncpasswd.c
  kasmpasswd.c)

target_link_libraries(vexovncpasswd crypt)

install(TARGETS vexovncpasswd DESTINATION ${BIN_DIR})
