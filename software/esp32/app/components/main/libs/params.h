#ifndef H_PARAMS
#define H_PARAMS

#include <string>
//using esp-idf api, since it's sharing storage with other modules
#include "nvs_flash.h"
#include "nvs.h"

namespace params {

    void IRAM_ATTR init();
    void IRAM_ATTR writeParam(std::string name, void* data, size_t len);
    void IRAM_ATTR writeParam(std::string name, uint32_t data);
    void IRAM_ATTR store();
    // int readParam(std::string name, void* data, void* def_val, size_t len, size_t defval_len);
    int IRAM_ATTR readParam(std::string name, uint32_t* data, uint32_t def_val);
    uint32_t IRAM_ATTR readParam(std::string name, uint32_t def_val);

    nvs_handle_t nvshdl;
    SemaphoreHandle_t xNvsMutex = NULL;
};

void params::init() {
    xNvsMutex = xSemaphoreCreateMutex();
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        printf("Erasing NVS flash...\n");
    }
    ESP_ERROR_CHECK( err );
    err = nvs_open("modemstr", NVS_READWRITE, &nvshdl);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
}

void params::writeParam(std::string name, void* data, size_t len) {
    if(!xSemaphoreTake (xNvsMutex, portMAX_DELAY)) { return; }
    if(nvs_set_blob(nvshdl, name.c_str(), data, len) != ESP_OK) {
        printf("PARAMS WRITE FAILED!\n");
    }
    xSemaphoreGive (xNvsMutex);
}

void params::writeParam(std::string name, uint32_t data) {
    if(!xSemaphoreTake (xNvsMutex, portMAX_DELAY)) { return; }
    if(nvs_set_u32(nvshdl, name.c_str(), data) != ESP_OK) {
        printf("PARAMS WRITE FAILED!\n");
    }
    xSemaphoreGive (xNvsMutex);
}

void params::store() {
    if(!xSemaphoreTake (xNvsMutex, portMAX_DELAY)) { return; }
    if(nvs_commit(nvshdl) != ESP_OK) {
        printf("PARAMS COMMIT FAILED!\n");
    }
    xSemaphoreGive (xNvsMutex);
}

// int params::readParam(std::string name, void* data, void* def_val, size_t len, size_t defval_len) {
//     size_t l = 0;
//     int err = nvs_get_blob(nvshdl, name.c_str(), NULL, &l);
//     printf("RD PARAM %s, LEN %d, RET %d\n", name.c_str(), l, err);
//     if(err != ESP_OK) {
// 		if(err == ESP_ERR_NVS_NOT_FOUND) {
// 			memcpy(data, def_val, defval_len);
// 			return defval_len;
// 		} else {
// 			printf("PARAMS LEN READ FAILED!\n");
// 			return 0;
// 		}
// 	}
//     err = nvs_get_blob(nvshdl, name.c_str(), data, &l);
//     printf("RD2 PARAM %s, LEN %d, RET %d\n", name.c_str(), l, err);
//     if(err != ESP_OK) {
//         if(err == ESP_ERR_NVS_NOT_FOUND) {
//             memcpy(data, def_val, defval_len);
//             return defval_len;
//         } else {
//             printf("PARAMS READ FAILED!\n");
//             return 0;
//         }
//     }
//     return l;
// }

int params::readParam(std::string name, uint32_t* data, uint32_t def_val) {
    *data = def_val;
    if(!xSemaphoreTake (xNvsMutex, portMAX_DELAY)) { return -2; }
    int err = nvs_get_u32(nvshdl, name.c_str(), data);
    if(err != ESP_OK) {
        if(err != ESP_ERR_NVS_NOT_FOUND) {
            printf("PARAMS READ FAILED(%d)!\n", err);
            xSemaphoreGive (xNvsMutex);
            return -2;
        } else {
            xSemaphoreGive (xNvsMutex);
            return -1;
        }
    }
    xSemaphoreGive (xNvsMutex);
    return 0;
}

uint32_t params::readParam(std::string name, uint32_t def_val) {
    uint32_t ret = def_val;
    if(!xSemaphoreTake (xNvsMutex, portMAX_DELAY)) { return ret; }
    int err = nvs_get_u32(nvshdl, name.c_str(), &ret);
    if(err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        printf("PARAMS READ FAILED(%d)!\n", err);
    }
    xSemaphoreGive (xNvsMutex);
    return ret;
}



#endif
