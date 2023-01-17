#include "timer.h"

chl_timer::chl_timer(int num) {
     assert(num >= 0 || num <= 3);
    _number = num;
    _timer = _number % 2;
    _group = _number - _timer;
    _timer_cb = nullptr;
    if(_timer == 0) {
        REG_SET_BIT(TIMG_INT_CLR_TIMERS_REG(_group), TIMG_T0_INT_CLR);
    } else {
        REG_SET_BIT(TIMG_INT_CLR_TIMERS_REG(_group), TIMG_T1_INT_CLR);
    }
    if(esp_intr_alloc(((_group == 0) ? ((_timer == 0) ? ETS_TG0_T0_LEVEL_INTR_SOURCE : ETS_TG0_T1_LEVEL_INTR_SOURCE) : ((_timer == 0) ? ETS_TG1_T0_LEVEL_INTR_SOURCE : ETS_TG1_T1_LEVEL_INTR_SOURCE)), ESP_INTR_FLAG_IRAM, _timer_intr_hdlr, this, &_timer_intr_hdl) != ESP_OK) {
        printf("TIMER%d ERROR: ESP INTR ALLOC FAIL\r\n", _number);
        return;
    }
    if(_timer == 0) {
        REG_SET_FIELD(TIMG_INT_ENA_TIMERS_REG(_group), TIMG_T0_INT_ENA, 1);
    } else {
        REG_SET_FIELD(TIMG_INT_ENA_TIMERS_REG(_group), TIMG_T1_INT_ENA, 1);
    }
}

chl_timer::~chl_timer() {
    esp_intr_free(_timer_intr_hdl);
    if(_timer == 0) {
        REG_SET_FIELD(TIMG_INT_ENA_TIMERS_REG(_group), TIMG_T0_INT_ENA, 0);
    } else {
        REG_SET_FIELD(TIMG_INT_ENA_TIMERS_REG(_group), TIMG_T1_INT_ENA, 0);
    }
}

void chl_timer::configurePrescaler(uint16_t prescaler) {
    if(_timer == 0) {
        REG_SET_FIELD(TIMG_T0CONFIG_REG(_group), TIMG_T0_DIVIDER, prescaler);
    } else {
        REG_SET_FIELD(TIMG_T1CONFIG_REG(_group), TIMG_T1_DIVIDER, prescaler);
    }
}

void chl_timer::configureTimer(bool dir, bool autoreload, uint64_t default_val, uint64_t cmp_val) {
    if(_timer == 0) {
        REG_SET_FIELD(TIMG_T0CONFIG_REG(_group), TIMG_T0_INCREASE, dir ? 1 : 0);
        REG_SET_FIELD(TIMG_T0CONFIG_REG(_group), TIMG_T0_AUTORELOAD, autoreload ? 1 : 0);
        REG_WRITE(TIMG_T0LOADLO_REG(_group), default_val & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T0LOADHI_REG(_group), (default_val & 0xFFFFFFFF00000000ULL) >> 32);
        REG_WRITE(TIMG_T0ALARMLO_REG(_group), cmp_val & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T0ALARMHI_REG(_group), (cmp_val & 0xFFFFFFFF00000000ULL) >> 32);
    } else {
        REG_SET_FIELD(TIMG_T1CONFIG_REG(_group), TIMG_T1_INCREASE, dir ? 1 : 0);
        REG_SET_FIELD(TIMG_T1CONFIG_REG(_group), TIMG_T1_AUTORELOAD, autoreload ? 1 : 0);
        REG_WRITE(TIMG_T1LOADLO_REG(_group), default_val & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T1LOADHI_REG(_group), (default_val & 0xFFFFFFFF00000000ULL) >> 32);
        REG_WRITE(TIMG_T1ALARMLO_REG(_group), cmp_val & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T1ALARMHI_REG(_group), (cmp_val & 0xFFFFFFFF00000000ULL) >> 32);
    }
}

void chl_timer::setInterruptEnabled(bool enabled) {
    if(_timer == 0) {
        REG_SET_FIELD(TIMG_T0CONFIG_REG(_group), TIMG_T0_LEVEL_INT_EN, enabled ? 1 : 0);
        REG_SET_FIELD(TIMG_T0CONFIG_REG(_group), TIMG_T0_ALARM_EN, enabled ? 1 : 0);
    } else {
        REG_SET_FIELD(TIMG_T1CONFIG_REG(_group), TIMG_T1_LEVEL_INT_EN, enabled ? 1 : 0);
        REG_SET_FIELD(TIMG_T1CONFIG_REG(_group), TIMG_T1_ALARM_EN, enabled ? 1 : 0);
    }
}

void chl_timer::setTimerRunning(bool start) {
    if(_timer == 0) {
        REG_SET_FIELD(TIMG_T0CONFIG_REG(_group), TIMG_T0_EN, start ? 1 : 0);
    } else {
        REG_SET_FIELD(TIMG_T1CONFIG_REG(_group), TIMG_T1_EN, start ? 1 : 0);
    }
}

void chl_timer::forceLoadVal(uint64_t val) {
    if(_timer == 0) {
        REG_WRITE(TIMG_T0UPDATE_REG(_group), 1);
        uint64_t prev_load = (REG_READ(TIMG_T0LO_REG(_group)) | ((uint64_t)REG_READ(TIMG_T0HI_REG(_group)) << 32));
        REG_WRITE(TIMG_T0LOADLO_REG(_group), val & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T0LOADHI_REG(_group), (val & 0xFFFFFFFF00000000ULL) >> 32);
        REG_WRITE(TIMG_T0LOAD_REG(_group), 1);
        REG_WRITE(TIMG_T0LOADLO_REG(_group), prev_load & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T0LOADHI_REG(_group), (prev_load & 0xFFFFFFFF00000000ULL) >> 32);
    } else {
        REG_WRITE(TIMG_T1UPDATE_REG(_group), 1);
        uint64_t prev_load =  (REG_READ(TIMG_T1LO_REG(_group)) | ((uint64_t)REG_READ(TIMG_T1HI_REG(_group)) << 32));
        REG_WRITE(TIMG_T1LOADLO_REG(_group), val & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T1LOADHI_REG(_group), (val & 0xFFFFFFFF00000000ULL) >> 32);
        REG_WRITE(TIMG_T1LOAD_REG(_group), 1);
        REG_WRITE(TIMG_T1LOADLO_REG(_group), prev_load & 0xFFFFFFFFULL);
        REG_WRITE(TIMG_T1LOADHI_REG(_group), (prev_load & 0xFFFFFFFF00000000ULL) >> 32);
    }
    getCurrTimerVal();
}

void chl_timer::forceReloadDefVal() {
    if(_timer == 0) {
        REG_WRITE(TIMG_T0LOAD_REG(_group), 1);
    } else {
        REG_WRITE(TIMG_T1LOAD_REG(_group), 1);
    }
    getCurrTimerVal();
}

uint64_t chl_timer::getCurrTimerVal() {
    if(_timer == 0) {
        REG_WRITE(TIMG_T0UPDATE_REG(_group), 1);
        return (REG_READ(TIMG_T0LO_REG(_group)) | ((uint64_t)REG_READ(TIMG_T0HI_REG(_group)) << 32));
    } else {
        REG_WRITE(TIMG_T1UPDATE_REG(_group), 1);
        return (REG_READ(TIMG_T1LO_REG(_group)) | ((uint64_t)REG_READ(TIMG_T1HI_REG(_group)) << 32));
    }
}

void chl_timer::setCallback(void (*callback)(void*), void* context) {
    _timer_cb = callback;
    _timer_cb_context = context;
}

void chl_timer::_timer_intr_hdlr(void* arg) {
    chl_timer* _this = (chl_timer*) arg;
    while(1) {
        uint32_t curr_intr = REG_READ(TIMG_INT_RAW_TIMERS_REG(_this->_group));
        if((curr_intr & (_this->_timer == 0 ? (TIMG_T0_INT_RAW) : (TIMG_T1_INT_RAW))) == 0) {
            break;
        }
        REG_WRITE(TIMG_INT_CLR_TIMERS_REG(_this->_group), (curr_intr & (_this->_timer == 0 ? (TIMG_T0_INT_RAW) : (TIMG_T1_INT_RAW))));
        if(_this->_timer_cb != nullptr) {
            _this->_timer_cb(_this->_timer_cb_context);
        }
        if(_this->_timer == 0) {
            REG_SET_FIELD(TIMG_T0CONFIG_REG(_this->_group), TIMG_T0_ALARM_EN, 1);
        } else {
            REG_SET_FIELD(TIMG_T1CONFIG_REG(_this->_group), TIMG_T1_ALARM_EN, 1);
        }
    }
}
