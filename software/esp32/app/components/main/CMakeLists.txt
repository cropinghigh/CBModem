idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ""
                    REQUIRES "customhwlibs_esp32" "customdsp" "esp_timer" "nvs_flash" "driver")

target_compile_options(${COMPONENT_LIB} PRIVATE "-std=gnu++20" "-mtext-section-literals")
