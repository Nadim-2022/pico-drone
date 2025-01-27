add_executable(${ProjectName}
        main.cpp
        critical_section.cpp
        Fmutex.cpp
        Fmutex.h
        blinker.cpp
        blinker.h

#        ipstack/IPStack.cpp
#        ipstack/IPStack.h
        ipstack/lwipopts.h
        ipstack/tls_common.c
        ipstack/picow_tls_client.c
        ipstack/lwipopts_tls.h

        i2c/PicoI2C.cpp
        i2c/PicoI2C.h

        uart/PicoOsUart.cpp
        uart/PicoOsUart.h
        src/SharedResources.cpp
        src/SharedResources.h

        src/Network/NetworkClass.cpp
        src/Network/NetworkClass.h


        ${MQTT_SOURCES}

        ipstack/dhcpserver.c
        ipstack/dhcpserver.h
        ipstack/dnsserver.c
        ipstack/dnsserver.h
        ipstack/picow_access_point.c

        #EEPROM
        src/eeprom/eeprom.cpp
        src/eeprom/eeprom.h
        src/MQTT/MQTTClient.h
        src/MQTT/MQTTClient.cpp
        src/RTC/RTC.cpp
        src/RTC/RTC.h


)

target_include_directories(${ProjectName} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
        modbus
        uart
        i2c
        display
        ipstack
        paho.mqtt.embedded-c/MQTTPacket/src
        paho.mqtt.embedded-c/MQTTClient/src
        ${CMAKE_CURRENT_LIST_DIR}/ipstack/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/ipstack/dnsserver
        ${PICO_LWIP_CONTRIB_PATH}/apps/httpd
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)

target_compile_definitions(${ProjectName} PRIVATE
        WIFI_SSID=\"$ENV{WIFI_SSID}\"
        WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"
        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
        #PICO_CYW43_ARCH_DEFAULT_COUNTRY_CODE=CYW43_COUNTRY_FINLAND
)

target_link_libraries(${ProjectName} 
        pico_stdlib
        hardware_i2c
        FreeRTOS-Kernel-Heap4
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_mbedtls
        pico_mbedtls
        pico_lwip_http
        pico_freertos_httpd_content
        pico_lwip_mqtt
        #pico_cyw43_arch_lwip_threadsafe_background

)

# Ignore warnings from lwip code
set_source_files_properties(
        ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
        PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-result"
)

pico_add_extra_outputs(${ProjectName})
pico_add_library(pico_freertos_httpd_content NOFLAG)


# Disable usb output, enable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
