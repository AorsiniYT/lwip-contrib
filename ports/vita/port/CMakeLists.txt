cmake_minimum_required(VERSION 3.10)

# lwIP port for PSVita
set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ext/lwip)

# Include directories
include_directories(${LWIP_DIR}/src/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files for PSVita port
set(lwip_port_src
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_arch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/strings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unistd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/netinet_in.c
    ${CMAKE_CURRENT_SOURCE_DIR}/socket.c
)

# Add the port library
add_library(lwip_port STATIC ${lwip_port_src})

# Link against VitaSDK libraries
target_link_libraries(lwip_port
    SceNet_stub
    SceKernel_stub
    SceSysmem_stub
)

# Set compile definitions for PSVita
target_compile_definitions(lwip_port PRIVATE
    __PSVITA__
    VITASDK
    LWIP_IPV6=0
)