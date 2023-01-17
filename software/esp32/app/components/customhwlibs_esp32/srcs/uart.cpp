#include "uart.h"

chl_uart_dma::chl_uart_dma(int num, int baudrate, int rxdgpio, int txdgpio) {
    assert(num == 0 || num == 1);
    _number = num;
    _baudrate = baudrate;
    _rxdgpio = rxdgpio;
    _txdgpio = txdgpio;
    _xTxLinksQueue = xQueueCreate(DMA_UART_TX_QUEUE_CNT, sizeof(lldesc_t));
    if(_xTxLinksQueue == NULL) {
        printf("UART%d DMA ERROR: NOT ENOUGH MEMORY FOR TX LINKS QUEUE\r\n", _number);
        return;
    }
    _tx_linkqueue_mtx = xSemaphoreCreateMutex();
    if(_tx_linkqueue_mtx == NULL) {
        printf("UART%d DMA ERROR: NOT ENOUGH MEMORY FOR TX LINKS QUEUE MUTEX\r\n", _number);
        return;
    }
    _rx_streambuffer_mtx = xSemaphoreCreateMutex();
    if(_rx_streambuffer_mtx == NULL) {
        printf("UART%d DMA ERROR: NOT ENOUGH MEMORY FOR RX STREAM BUFFER MUTEX\r\n", _number);
        return;
    }
    _xRxStreamBuffer = xStreamBufferCreate(DMA_UART_RX_BUFF_SIZE, 1);
    if(_xRxStreamBuffer == NULL) {
        printf("UART%d DMA ERROR: NOT ENOUGH MEMORY FOR RX STREAM BUFFER\r\n", _number);
        return;
    }
    _chl_uhci_dma_rx_buff = (uint8_t*)heap_caps_malloc(DMA_UART_RX_BUFF_SIZE, MALLOC_CAP_DMA);
    if(_chl_uhci_dma_rx_buff == NULL) {
        printf("UART%d DMA ERROR: NOT ENOUGH MEMORY FOR RX BUFFER\r\n", _number);
        return;
    }
    _chl_uhci_dma_inlinks = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t)*DMA_UART_RX_BUFF_CNT, MALLOC_CAP_DMA);
    if(_chl_uhci_dma_inlinks == NULL) {
        printf("UART%d DMA ERROR: NOT ENOUGH MEMORY FOR RX INPUT LINKS\r\n", _number);
        return;
    }
    _chl_uhci_dma_outlink = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t), MALLOC_CAP_DMA);
    if(_chl_uhci_dma_outlink == NULL) {
        printf("UART%d DMA ERROR: NOT ENOUGH MEMORY FOR OUTPUT LINK\r\n", _number);
        return;
    }

    _reset_module();

    if(esp_intr_alloc((_number == 0) ? ETS_UHCI0_INTR_SOURCE : ETS_UHCI1_INTR_SOURCE, ESP_INTR_FLAG_IRAM, _uhci_intr_hdlr, this, &_uhci_intr_hdl) != ESP_OK) {
        printf("UART%d DMA ERROR: ESP INTR ALLOC FAIL\r\n", _number);
        return;
    }
    REG_WRITE(UHCI_INT_CLR_REG(_number), 0b1111111111111111);

    for(int i = 0; i < DMA_UART_RX_BUFF_CNT; i++) {
        _chl_uhci_dma_inlinks[i].eof = 1;
        _chl_uhci_dma_inlinks[i].owner = 1;
        _chl_uhci_dma_inlinks[i].size = DMA_UART_RX_BUFF_SIZE/DMA_UART_RX_BUFF_CNT;
        _chl_uhci_dma_inlinks[i].length = 0;
        _chl_uhci_dma_inlinks[i].buf = _chl_uhci_dma_rx_buff+((DMA_UART_RX_BUFF_SIZE/DMA_UART_RX_BUFF_CNT)*i);
        _chl_uhci_dma_inlinks[i].empty = (uint32_t)&_chl_uhci_dma_inlinks[(i+1)%DMA_UART_RX_BUFF_CNT];
    }
    _curr_inlink = &_chl_uhci_dma_inlinks[0];
    _set_intr_start_rx();
}

chl_uart_dma::~chl_uart_dma() {
    esp_intr_free(_uhci_intr_hdl);
    periph_module_disable((_number == 0) ? PERIPH_UHCI0_MODULE : PERIPH_UHCI1_MODULE);
    heap_caps_free(_chl_uhci_dma_rx_buff);
    heap_caps_free(_chl_uhci_dma_inlinks);
    heap_caps_free(_chl_uhci_dma_outlink);
    vQueueDelete(_xTxLinksQueue);
    vSemaphoreDelete(_tx_linkqueue_mtx);
    vSemaphoreDelete(_rx_streambuffer_mtx);
    vStreamBufferDelete(_xRxStreamBuffer);
}

int chl_uart_dma::transmit_bytes(uint8_t* buf, unsigned int len, bool block) {
    assert(esp_ptr_dma_capable(buf));
    if(xSemaphoreTake(_tx_linkqueue_mtx, block ? portMAX_DELAY : 0) != pdTRUE) {
        return ESP_FAIL;
    }
    lldesc_t new_outlink_descr;
    new_outlink_descr.eof = 1;
    new_outlink_descr.owner = 1;
    new_outlink_descr.size = len;
    new_outlink_descr.length = len;
    new_outlink_descr.buf = buf;
    new_outlink_descr.empty = 0;
    if(xQueueSendToBack(_xTxLinksQueue, &new_outlink_descr, block ? portMAX_DELAY : 0) != pdTRUE) {
        xSemaphoreGive(_tx_linkqueue_mtx);
        return ESP_FAIL;
    }
    if(!REG_GET_BIT(UHCI_INT_ENA_REG(_number), (UHCI_OUT_EOF_INT_ENA | UHCI_OUT_DSCR_ERR_INT_ENA))) {
        _set_intr_start_tx();
    }
    xSemaphoreGive(_tx_linkqueue_mtx);
    return ESP_OK;
}

int chl_uart_dma::read_bytes(uint8_t* buf, unsigned int len, bool block) {
    if(xSemaphoreTake(_rx_streambuffer_mtx, block ? portMAX_DELAY : 0) != pdTRUE) {
        return -1;
    }
    int rlen = xStreamBufferReceive(_xRxStreamBuffer, buf, len, block ? portMAX_DELAY : 0);
    xSemaphoreGive(_rx_streambuffer_mtx);
    return rlen;
}

int chl_uart_dma::getTxQueueCount() {
    return uxQueueMessagesWaiting(_xTxLinksQueue);
}

int chl_uart_dma::getRxByteCount() {
    return xStreamBufferBytesAvailable(_xRxStreamBuffer);
}

void chl_uart_dma::_uhci_intr_hdlr(void* arg) {
    chl_uart_dma* _this = (chl_uart_dma*)arg;
    portBASE_TYPE contsw_req = false;
    while(1) {
        uint32_t curr_intr = REG_READ(UHCI_INT_RAW_REG(_this->_number));
        if(curr_intr == 0) {
            break;
        }
        REG_WRITE(UHCI_INT_CLR_REG(_this->_number), curr_intr);
        if((curr_intr & UHCI_IN_DONE_INT_RAW) || (curr_intr & UHCI_IN_SUC_EOF_INT_RAW)) {
            //no flow control; all unwritten data due to full buffer will be lost
            xStreamBufferSendFromISR(_this->_xRxStreamBuffer, (const void*)_this->_curr_inlink->buf, _this->_curr_inlink->length, &contsw_req);
            _this->_curr_inlink->eof = 0;
            _this->_curr_inlink->owner = 1;
            _this->_curr_inlink = (lldesc_t*)_this->_curr_inlink->empty;
        }
        if((curr_intr & UHCI_OUT_EOF_INT_RAW) || (curr_intr & UHCI_OUT_DSCR_ERR_INT_RAW)) {
            if(xQueueReceiveFromISR(_this->_xTxLinksQueue, (_this->_chl_uhci_dma_outlink), &contsw_req) == pdTRUE) {
                REG_SET_BIT(UHCI_DMA_OUT_LINK_REG(_this->_number), UHCI_OUTLINK_START);
            } else {
                REG_CLR_BIT(UHCI_INT_ENA_REG(_this->_number), (UHCI_OUT_EOF_INT_RAW | UHCI_OUT_DSCR_ERR_INT_ENA));
            }
        }
    }
    if(contsw_req) {
        portYIELD_FROM_ISR();
    }
}

void chl_uart_dma::_set_intr_start_tx() {
    if(xQueueReceive(_xTxLinksQueue, _chl_uhci_dma_outlink, 0) != pdTRUE) {
        _chl_uhci_dma_outlink->eof = 1;
        _chl_uhci_dma_outlink->owner = 1;
        _chl_uhci_dma_outlink->size = 0;
        _chl_uhci_dma_outlink->length = 0;
        _chl_uhci_dma_outlink->buf = 0;
        _chl_uhci_dma_outlink->empty = 0;
    }
    REG_SET_BIT(UHCI_INT_ENA_REG(_number), (UHCI_OUT_EOF_INT_ENA | UHCI_OUT_DSCR_ERR_INT_ENA));
    REG_SET_BIT(UHCI_DMA_OUT_LINK_REG(_number), UHCI_OUTLINK_START);
}

void chl_uart_dma::_set_intr_start_rx() {
    REG_SET_BIT(UHCI_INT_ENA_REG(_number), (UHCI_IN_DONE_INT_ENA | UHCI_IN_SUC_EOF_INT_ENA ));
    REG_SET_BIT(UHCI_DMA_IN_LINK_REG(_number), UHCI_INLINK_START);
}

void chl_uart_dma::_reset_module() {
    _config_gpio();
    //perform default UART configuration: 1 stop bit; 8 data bits; no flow control; 96 bytes FIFO threshold; using APB clock(assuming it's constant)
    REG_WRITE(UART_CONF0_REG(_number), UART_TICK_REF_ALWAYS_ON | (0b01 << UART_STOP_BIT_NUM_S) | (0b11 << UART_BIT_NUM_S));
    REG_WRITE(UART_CONF1_REG(_number), (0x60 << UART_RXFIFO_FULL_THRHD_S) | (0x60 << UART_TXFIFO_EMPTY_THRHD_S));
    int intergral_divider = ((APB_CLK_FREQ+_baudrate/2) / _baudrate) & UART_CLKDIV_V;
    int decimal_divider = ((((APB_CLK_FREQ+_baudrate/2) % _baudrate)*10 + _baudrate/2) / _baudrate) & UART_CLKDIV_FRAG_V;
    REG_WRITE(UART_CLKDIV_REG(_number), (intergral_divider << UART_CLKDIV_S) | (decimal_divider << UART_CLKDIV_FRAG_S));
    REG_WRITE(UART_FLOW_CONF_REG(_number), 0);
    REG_WRITE(UART_SLEEP_CONF_REG(_number), (0xF0 << UART_ACTIVE_THRESHOLD_S));
    REG_WRITE(UART_IDLE_CONF_REG(_number), (0x00A << UART_TX_BRK_NUM_S) | (0x010 << UART_TX_IDLE_NUM_S) | (0x1FF << UART_RX_IDLE_THRHD_S));
    REG_WRITE(UART_RS485_CONF_REG(_number), 0);
    REG_WRITE(UART_AUTOBAUD_REG(_number), (0x010 << UART_GLITCH_FILT_S));
    REG_WRITE(UART_MEM_CONF_REG(_number), (0x01 << UART_RX_SIZE_S) | (0x01 << UART_TX_SIZE_S));
    REG_WRITE(UART_INT_ENA_REG(_number), 0);

    //DMA config
    periph_module_enable((_number == 0) ? PERIPH_UHCI0_MODULE : PERIPH_UHCI1_MODULE); //enable UDMA clk(sets DPORT_UHCIx_RST low and DPORT_UHCIx_CLK_EN high)
    REG_WRITE(UHCI_INT_ENA_REG(_number), 0);
    REG_SET_BIT(UHCI_CONF0_REG(_number), UHCI_CLK_EN);
    REG_WRITE(UHCI_CONF0_REG(_number), UHCI_CLK_EN | UHCI_IN_RST | UHCI_OUT_RST | ((_number == 0) ? UHCI_UART0_CE : UHCI_UART1_CE) | UHCI_UART_IDLE_EOF_EN);
    REG_CLR_BIT(UHCI_CONF0_REG(_number), UHCI_IN_RST | UHCI_OUT_RST);
    REG_WRITE(UHCI_CONF1_REG(_number), (100 << UHCI_DMA_INFIFO_FULL_THRS_S));
    REG_WRITE(UHCI_ESCAPE_CONF_REG(_number), 0);

    REG_WRITE(UHCI_DMA_IN_LINK_REG(_number), ((uint32_t)(&(_chl_uhci_dma_inlinks[0])) << UHCI_INLINK_ADDR_S) & UHCI_INLINK_ADDR_M);
    REG_WRITE(UHCI_DMA_OUT_LINK_REG(_number), ((uint32_t)(_chl_uhci_dma_outlink) << UHCI_OUTLINK_ADDR_S) & UHCI_OUTLINK_ADDR_M);
}
void chl_uart_dma::_config_gpio() {
    if(_number == 0 && _rxdgpio == 3 && _txdgpio == 1) {
        //Using IOMUX connection
        chl_gpio_iomux_select_func(_rxdgpio, FUNC_U0RXD_U0RXD);
        chl_gpio_iomux_select_func(_txdgpio, FUNC_U0TXD_U0TXD);
        chl_gpio_connect_out(_txdgpio, U0TXD_OUT_IDX, false);
        chl_gpio_connect_in(_rxdgpio, U0RXD_IN_IDX, false, true);
    } else {
        chl_gpio_iomux_select_func(_rxdgpio, PIN_FUNC_GPIO);
        chl_gpio_iomux_select_func(_txdgpio, PIN_FUNC_GPIO);
        chl_gpio_set_direction(_rxdgpio, true, false, false, false, false);
        chl_gpio_set_direction(_txdgpio, false, true, false, false, false);
        chl_gpio_connect_out(_txdgpio, (_number == 0) ? U0TXD_OUT_IDX : U1TXD_OUT_IDX, false);
        chl_gpio_connect_in(_rxdgpio, (_number == 0) ? U0RXD_IN_IDX : U1RXD_IN_IDX, false, false);
    }
}
