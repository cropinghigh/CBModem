#ifndef H_PARAMS
#define H_PARAMS

#include <string>
//using esp-idf api, since it's sharing storage with other modules
#include "nvs_flash.h"
#include "nvs.h"

namespace params {

    void init();
    void writeParam(std::string name, void* data, size_t len);
    void writeParam(std::string name, uint32_t data);
    void store();
    int readParam(std::string name, void* data, void* def_val, size_t len, size_t defval_len);
    uint32_t readParam(std::string name, uint32_t def_val);

    nvs_handle_t nvshdl;
};

void params::init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    err = nvs_open("modemstr", NVS_READWRITE, &nvshdl);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
}

void params::writeParam(std::string name, void* data, size_t len) {
    if(nvs_set_blob(nvshdl, name.c_str(), data, len) != ESP_OK) {
        printf("PARAMS WRITE FAILED!\n");
    }
}

void params::writeParam(std::string name, uint32_t data) {
    if(nvs_set_u32(nvshdl, name.c_str(), data) != ESP_OK) {
        printf("PARAMS WRITE FAILED!\n");
    }
}

void params::store() {
    if(nvs_commit(nvshdl) != ESP_OK) {
        printf("PARAMS COMMIT FAILED!\n");
    }
}

int params::readParam(std::string name, void* data, void* def_val, size_t len, size_t defval_len) {
    size_t l = 0;
    int err = nvs_get_blob(nvshdl, name.c_str(), NULL, &l);
    if(err != ESP_OK) {
		if(err == ESP_ERR_NVS_NOT_FOUND) {
			memcpy(data, def_val, defval_len);
			return defval_len;
		} else {
			printf("PARAMS LEN READ FAILED!\n");
			return 0;
		}
	}
    err = nvs_get_blob(nvshdl, name.c_str(), data, &l);
    if(err != ESP_OK) {
        if(err == ESP_ERR_NVS_NOT_FOUND) {
            memcpy(data, def_val, defval_len);
            return defval_len;
        } else {
            printf("PARAMS READ FAILED!\n");
            return 0;
        }
    }
    return l;
}

uint32_t params::readParam(std::string name, uint32_t def_val) {
    uint32_t ret = def_val;
    int err = nvs_get_u32(nvshdl, name.c_str(), &ret);
    if(err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        printf("PARAMS READ FAILED!\n");
    }
    return ret;
}



#endif
