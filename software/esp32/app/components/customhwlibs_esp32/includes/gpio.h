#pragma once
#include <soc/gpio_periph.h>
#include <soc/rtc_periph.h>

//WARNING! NO THREAD SAFETY! NO IDIOT PROTECTION! BE CAREFUL!
//TODO: interrupts configuration; RTC functions; sleep mode configuration; hold; clock output; drive strength

static const uint32_t iomux_regs[40] = {
    IO_MUX_GPIO0_REG,
    IO_MUX_GPIO1_REG,
    IO_MUX_GPIO2_REG,
    IO_MUX_GPIO3_REG,
    IO_MUX_GPIO4_REG,
    IO_MUX_GPIO5_REG,
    IO_MUX_GPIO6_REG,
    IO_MUX_GPIO7_REG,
    IO_MUX_GPIO8_REG,
    IO_MUX_GPIO9_REG,
    IO_MUX_GPIO10_REG,
    IO_MUX_GPIO11_REG,
    IO_MUX_GPIO12_REG,
    IO_MUX_GPIO13_REG,
    IO_MUX_GPIO14_REG,
    IO_MUX_GPIO15_REG,
    IO_MUX_GPIO16_REG,
    IO_MUX_GPIO17_REG,
    IO_MUX_GPIO18_REG,
    IO_MUX_GPIO19_REG,
    IO_MUX_GPIO20_REG,
    IO_MUX_GPIO21_REG,
    IO_MUX_GPIO22_REG,
    IO_MUX_GPIO23_REG,
    IO_MUX_GPIO24_REG,
    IO_MUX_GPIO25_REG,
    IO_MUX_GPIO26_REG,
    IO_MUX_GPIO27_REG,
    0,
    0,
    0,
    0,
    IO_MUX_GPIO32_REG,
    IO_MUX_GPIO33_REG,
    IO_MUX_GPIO34_REG,
    IO_MUX_GPIO35_REG,
    IO_MUX_GPIO36_REG,
    IO_MUX_GPIO37_REG,
    IO_MUX_GPIO38_REG,
    IO_MUX_GPIO39_REG
};

inline void chl_gpio_iomux_select_func(int gpio, uint32_t func) {
    if(rtc_io_num_map[gpio] != -1) {
        REG_CLR_BIT(rtc_io_desc[rtc_io_num_map[gpio]].reg, rtc_io_desc[rtc_io_num_map[gpio]].mux); //if pin is an RTC pin, redirect it to the IOMUX
    }
    REG_SET_FIELD(iomux_regs[gpio], MCU_SEL, func);
}

inline void chl_gpio_set_direction(int gpio, bool input, bool output, bool od, bool pulldown, bool pullup) {
    REG_SET_FIELD(iomux_regs[gpio], FUN_IE, input);
    if(rtc_io_num_map[gpio] == -1) {
        REG_SET_FIELD(iomux_regs[gpio], FUN_PU, pullup);
        REG_SET_FIELD(iomux_regs[gpio], FUN_PD, pulldown);
    } else if(rtc_io_desc[rtc_io_num_map[gpio]].pullup != 0) {
        if(input) {
            REG_SET_BIT(rtc_io_desc[rtc_io_num_map[gpio]].reg, rtc_io_desc[rtc_io_num_map[gpio]].ie);
        } else {
            REG_CLR_BIT(rtc_io_desc[rtc_io_num_map[gpio]].reg, rtc_io_desc[rtc_io_num_map[gpio]].ie);
        }
        if(pullup) {
            REG_SET_BIT(rtc_io_desc[rtc_io_num_map[gpio]].reg, rtc_io_desc[rtc_io_num_map[gpio]].pullup);
        } else {
            REG_CLR_BIT(rtc_io_desc[rtc_io_num_map[gpio]].reg, rtc_io_desc[rtc_io_num_map[gpio]].pullup);
        }
        if(pulldown) {
            REG_SET_BIT(rtc_io_desc[rtc_io_num_map[gpio]].reg, rtc_io_desc[rtc_io_num_map[gpio]].pulldown);
        } else {
            REG_CLR_BIT(rtc_io_desc[rtc_io_num_map[gpio]].reg, rtc_io_desc[rtc_io_num_map[gpio]].pulldown);
        }
    }
    if(gpio < 32) {
        if(output) {
            REG_SET_BIT(GPIO_ENABLE_REG, (1 << gpio));
        } else {
            REG_CLR_BIT(GPIO_ENABLE_REG, (1 << gpio));
        }
    } else {
        if(output) {
            REG_SET_BIT(GPIO_ENABLE1_REG, (1 << (gpio-32)));
        } else {
            REG_CLR_BIT(GPIO_ENABLE1_REG, (1 << (gpio-32)));
        }
    }
    uint32_t gpio_reg = GPIO_PIN0_REG + gpio*0x4;
    if(od) {
        REG_SET_BIT(gpio_reg, GPIO_PIN0_PAD_DRIVER);
    } else {
        REG_CLR_BIT(gpio_reg, GPIO_PIN0_PAD_DRIVER);
    }
}

inline void chl_gpio_set_level(int gpio, bool level) {
    if(gpio < 32) {
        if(level) {
            REG_SET_BIT(GPIO_OUT_REG, (1 << gpio));
        } else {
            REG_CLR_BIT(GPIO_OUT_REG, (1 << gpio));
        }
    } else {
        if(level) {
            REG_SET_BIT(GPIO_OUT1_REG, (1 << (gpio-32)));
        } else {
            REG_CLR_BIT(GPIO_OUT1_REG, (1 << (gpio-32)));
        }
    }
}

inline bool chl_gpio_get_level(int gpio) {
    if(gpio < 32) {
        uint8_t reg_pos = gpio;
        return ((REG_READ(GPIO_IN_REG) & (1 << reg_pos)) >> reg_pos);
    } else {
        uint8_t reg_pos = gpio-32;
        return ((REG_READ(GPIO_IN1_REG) & (1 << reg_pos)) >> reg_pos);
    }
}

inline void chl_gpio_connect_out(int gpio, int out_sig_idx, bool invert) {
    uint32_t gpio_out_func_reg = GPIO_FUNC0_OUT_SEL_CFG_REG + gpio*0x4;
    REG_WRITE(gpio_out_func_reg, ((out_sig_idx << GPIO_FUNC0_OUT_SEL_S)&GPIO_FUNC0_OUT_SEL_M) | (invert << GPIO_FUNC0_OUT_INV_SEL_S));
}

inline void chl_gpio_connect_in(int gpio, int in_sig_idx, bool invert, bool iomux) {
    uint32_t gpio_in_func_reg = GPIO_FUNC0_IN_SEL_CFG_REG + in_sig_idx*0x4;
    REG_WRITE(gpio_in_func_reg, ((gpio << GPIO_FUNC0_IN_SEL_S)&GPIO_FUNC0_IN_SEL_M) | (invert << GPIO_FUNC0_IN_INV_SEL_S) | (iomux << GPIO_SIG0_IN_SEL_S));
}

inline void chl_rtc_gpio_enable_dacs(bool first, bool second) {
    if(first) {
        REG_WRITE(RTC_IO_PAD_DAC1_REG, (2 << RTC_IO_PDAC1_DRV_S) | (0 << RTC_IO_PDAC1_DAC_S) | RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_MUX_SEL | RTC_IO_PDAC1_DAC_XPD_FORCE);
    }
    if(second) {
        REG_WRITE(RTC_IO_PAD_DAC2_REG, (2 << RTC_IO_PDAC2_DRV_S) | (0 << RTC_IO_PDAC2_DAC_S) | RTC_IO_PDAC2_XPD_DAC | RTC_IO_PDAC2_MUX_SEL | RTC_IO_PDAC2_DAC_XPD_FORCE);
    }
}

inline void chl_rtc_gpio_disable_dacs(bool first, bool second) {
    if(first) {
        REG_WRITE(RTC_IO_PAD_DAC1_REG, (2 << RTC_IO_PDAC1_DRV_S) | (0 << RTC_IO_PDAC1_DAC_S));
    }
    if(second) {
        REG_WRITE(RTC_IO_PAD_DAC2_REG, (2 << RTC_IO_PDAC2_DRV_S) | (0 << RTC_IO_PDAC2_DAC_S));
    }
}

inline void chl_rtc_gpio_set_dac1_value(uint8_t val) {
    REG_SET_FIELD(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_DAC, val);
}

inline void chl_rtc_gpio_set_dac2_value(uint8_t val) {
    REG_SET_FIELD(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_DAC, val);
}
