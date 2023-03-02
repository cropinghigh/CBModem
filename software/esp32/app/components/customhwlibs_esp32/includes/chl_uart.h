#ifndef H_CHL_UART
#define H_CHL_UART
#include <soc/uart_periph.h>
#include <soc/uhci_periph.h>
#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <esp_system.h>
#include <esp_private/periph_ctrl.h>
#include <rom/lldesc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/stream_buffer.h>
#include <freertos/semphr.h>

#include <rom/ets_sys.h>

#include "chl_gpio.h"

#define DMA_UART_RX_BUFF_SIZE 1024
#define DMA_UART_RX_BUFF_CNT 16
#define DMA_UART_TX_QUEUE_CNT 16

class chl_uart_dma {
public:
    //only 0 and 1 supported; automatically starts receiving
    chl_uart_dma(int num, int baudrate, int rxdgpio, int txdgpio);
    ~chl_uart_dma();
    //Give specified buffer to the DMA and make it send it;possibly should work for multiple threads; buffer should be inside DMA memory range; returns ESP_OK or ESP_FAIL
    //buf - buffer in DMA access zone; len - size of data being transmitted; block - wait for queue to free, or immediately return fail
    int transmit_bytes(uint8_t* buf, unsigned int len, bool block);
    //Copy data from UART buffer to specified buffer; less effective than tx(data is copied 3 times, instead of 0); possibly should work for multiple threads; buffer could be anywhere; returns number of read bytes or -1
    //buf - buffer anywhere to copy data to; len - available size in the buffer; block - wait for len bytes or return as many as present
    int read_bytes(uint8_t* buf, unsigned int len, bool block);
    //Return count of items in the TX queue
    int getTxQueueCount();
    //Wait for the TX transactions to finish
    void waitForTxFinish();
    //Return count of bytes in the RX buffer
    int getRxByteCount();

private:
    int _number;
    int _rxdgpio, _txdgpio;
    int _baudrate;
    SemaphoreHandle_t _tx_linkqueue_mtx;
    SemaphoreHandle_t _rx_streambuffer_mtx;
    QueueHandle_t _xTxLinksQueue;
    StreamBufferHandle_t _xRxStreamBuffer;
    TaskHandle_t _tx_wait_task_hdl;
    intr_handle_t _uhci_intr_hdl;
    lldesc_t* _curr_inlink;
    uint8_t* _chl_uhci_dma_rx_buff; //DMA_UART_RX_BUFF_SIZE
    lldesc_t* _chl_uhci_dma_inlinks; //DMA_UART_RX_BUFF_CNT
    lldesc_t* _chl_uhci_dma_outlink;

    inline void _set_intr_start_tx();
    inline void _set_intr_start_rx();
    void _reset_module();
    void _config_gpio();
    static void IRAM_ATTR _uhci_intr_hdlr(void* arg);
};
#endif
