#include "cdsp_gen.h" 

cdsp_gen_sine_complex::cdsp_gen_sine_complex(float fs, float fr) {
    setFs(fs);
    setFr(fr);
}

void cdsp_gen_sine_complex::setFs(float fs) {
    _fs = fs;
    if(_fs != 0) {
        _incr = 2.0f*FL_PI*_fr/_fs;
    }
}

void cdsp_gen_sine_complex::setFr(float fr) {
    // _frlatch = fr;
    _fr = fr;
    if(_fs != 0) {
        _incr = 2.0f*FL_PI*_fr/_fs;
    }
}

int cdsp_gen_sine_complex::requestData(void* ctx, cdsp_complex_t* data, int samples_cnt) {
    cdsp_gen_sine_complex* _this = (cdsp_gen_sine_complex*) ctx;
    if(!_this->_running) {return -2;}
    for(int i = 0; i < samples_cnt; i++) {
        data[i].i = cosf(_this->_phase); //Should be quite slow, but performance is enough; replace with LUT later if required
        data[i].q = sinf(_this->_phase);
        _this->_phase += _this->_incr;
        while(_this->_phase > FL_PI) {
            _this->_phase -= 2.0f*FL_PI;
        }
        while(_this->_phase < -FL_PI) {
            _this->_phase += 2.0f*FL_PI;
        }
        // if(fabsf(_this->_phase - 0.0f) < 0.001f) {
        //     _this->_fr = _this->_frlatch; //Keep phase continuity
        //     if(_this->_fs != 0) {
        //         _this->_incr = 2.0f*FL_PI*_this->_fr/_this->_fs;
        //     }
        // }
    }
    return samples_cnt;
}


cdsp_gen_sine_float::cdsp_gen_sine_float(float fs, float fr) {
    setFs(fs);
    setFr(fr);
}

void cdsp_gen_sine_float::setFs(float fs) {
    _fs = fs;
    if(_fs != 0) {
        _incr = 2.0f*FL_PI*_fr/_fs;
    }
}

void cdsp_gen_sine_float::setFr(float fr) {
    _fr = fr;
    if(_fs != 0) {
        _incr = 2.0f*FL_PI*_fr/_fs;
    }
}

int cdsp_gen_sine_float::requestData(void* ctx, float* data, int samples_cnt) {
    cdsp_gen_sine_float* _this = (cdsp_gen_sine_float*) ctx;
    if(!_this->_running) {return -2;}
    for(int i = 0; i < samples_cnt; i++) {
        data[i] = cosf(_this->_phase); //Should be quite slow, but performance is enough; replace with LUT later if required
        _this->_phase += _this->_incr;
        while(_this->_phase >= 2.0f*FL_PI) {
            _this->_phase -= 2.0f*FL_PI;
        }
        while(_this->_phase <= -2.0f*FL_PI) {
            _this->_phase += 2.0f*FL_PI;
        }
    }
    return samples_cnt;
}
