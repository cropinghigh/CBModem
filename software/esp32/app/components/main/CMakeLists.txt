idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ""
                    REQUIRES "customhwlibs_esp32" "esp_timer")

target_compile_options(${COMPONENT_LIB} PRIVATE "-std=gnu++20")
