# Made because some esp-idf components(like ADC) are buggy or too slow & to learn a bit more about esp32 hardware
# ONLY supported chip is esp32, and ONLY one thread on single core; almost all locks are removed to improve performance

idf_component_register(SRC_DIRS "srcs/"
                    INCLUDE_DIRS "includes"
                    REQUIRES "customhwlibs_esp32")

target_compile_options(${COMPONENT_LIB} PRIVATE "-std=gnu++20" "-mtext-section-literals")
