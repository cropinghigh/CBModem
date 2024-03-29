#ifndef H_CHL_TIMER
#define H_CHL_TIMER
#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <soc/timer_group_reg.h>

#include <rom/ets_sys.h>

class chl_timer {
public:
    chl_timer(int num);
    ~chl_timer();
    void configurePrescaler(uint16_t prescaler);
    void configureTimer(bool dir, bool autoreload, uint64_t default_val, uint64_t cmp_val); //true-increment
    void setInterruptEnabled(bool enabled);
    void setTimerRunning(bool start);
    void setDefaultVal(uint64_t val);
    void setTimerVal(uint64_t cmp_val);
    void forceLoadVal(uint64_t val);
    void forceReloadDefVal();
    uint64_t getCurrTimerVal();
    uint64_t getDefaultTimerVal();
    void setCallback(void (*callback)(void*), void* context);
private:
    int _number;
    int _group;
    int _timer;
    intr_handle_t _timer_intr_hdl;
    void (*_timer_cb)(void*);
    void* _timer_cb_context;

    static void IRAM_ATTR _timer_intr_hdlr(void* arg);
};
#endif
