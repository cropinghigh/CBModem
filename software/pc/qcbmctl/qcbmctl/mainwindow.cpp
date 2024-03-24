#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), settings("Indir", "qcbmctl"), font("Monospace") {
    ui->setupUi(this);
    setCentralWidget(ui->tabWidget);

    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(15);
    font.setBold(true);
    statusbar_conn.setFont(font);
    statusbar_activity.setFont(font);
    ui->statusbar->setFont(font);
    ui->statusbar->addPermanentWidget(&statusbar_activity);
    ui->statusbar->addPermanentWidget(&statusbar_conn);
    ui->statusbar->clearMessage();
    statusbar_conn.setText("Disconected");

    ui->textBrowser_3->clear();
    QObject::connect(this, &MainWindow::do_insertPlainText, this, &MainWindow::on_do_insertPlainText);
    QObject::connect(this, &MainWindow::do_insertHTMLTRX, this, &MainWindow::on_do_insertHTMLTRX);
    QObject::connect(this, &MainWindow::do_sendingFinished, this, &MainWindow::on_sending_finished);
    QObject::connect(this, &MainWindow::do_updateActivity, this, &MainWindow::on_do_updateActivity);

    QObject::connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::on_pushButton_3_clicked);

    QObject::connect(&act_rx_tmr, &QTimer::timeout, this, &MainWindow::on_actrxtimer_timeout, Qt::QueuedConnection);
    QObject::connect(&act_tx_tmr, &QTimer::timeout, this, &MainWindow::on_acttxtimer_timeout, Qt::QueuedConnection);
    QObject::connect(&conn_time_tmr, &QTimer::timeout, this, &MainWindow::on_conntimetimer_timeout, Qt::QueuedConnection);

    conn_time_tmr.start(1000);

    if(settings.contains("param_freq")) {
        ui->spinBox_4->setValue(settings.value("param_freq").toInt());
    }
    if(settings.contains("param_mode")) {
        ui->comboBox_2->setCurrentIndex(settings.value("param_mode").toInt());
    }
    if(settings.contains("param_speed")) {
        ui->spinBox->setValue(settings.value("param_speed").toInt());
    }
    if(settings.contains("param_frspacing")) {
        ui->spinBox_2->setValue(settings.value("param_frspacing").toInt());
    }
    if(settings.contains("param_frcnt")) {
        ui->spinBox_3->setValue(settings.value("param_frcnt").toInt());
    }

    refreshActivity();
    on_pushButton_2_clicked();
}

MainWindow::~MainWindow() {
    if(conn_alive) {
        param_init = false;
        conn_alive = false;
        conn_thrd_res.waitForFinished();
    }
    delete ui;
}

void MainWindow::connThrdFunc(MainWindow* ctx) {
    MainWindow* _this = (MainWindow*) ctx;
    if(_this->conn_alive) {
        bool res = _this->modemPI.start();
        if(res) {
            _this->on_updateConStatus(CONN_STATUS_CONNECTING);
            //TODO: make reset switchable
            _this->conn_time = 0;
            _this->sent_pkts = 0;
            _this->rcvd_pkts = 0;
            cbmodem::pc_packet_interface::modes newmode;
            switch(_this->ui->comboBox_2->currentIndex()) {
                case 0:
                    newmode = cbmodem::pc_packet_interface::PC_PI_MODE_NORMAL_BFSK;
                    break;
                case 1:
                    newmode = cbmodem::pc_packet_interface::PC_PI_MODE_NORMAL_MFSK;
                    break;
                case 2:
                    newmode = cbmodem::pc_packet_interface::PC_PI_MODE_NORMAL_MSK;
                    break;
                default:
                    newmode = cbmodem::pc_packet_interface::PC_PI_MODE_SDR;
                    break;
            }
            _this->rx2_thrd_res = QtConcurrent::run(rx2ThrdFunc, _this);
            emit _this->do_updateActivity(true);
            _this->modemPI.change_mode(newmode, true);
            _this->modemPI.set_speed((float)_this->ui->spinBox->value(), (float)_this->ui->spinBox_2->value(), _this->ui->spinBox_3->value());
            emit _this->do_updateActivity(true);
            _this->modemPI.set_fr((float)_this->ui->spinBox_4->value());
            emit _this->do_updateActivity(true);
            _this->reloadParams();
            emit _this->do_updateActivity(true);
            _this->modemPI.start_rx();
            _this->on_updateConStatus(CONN_STATUS_CONNECTED);
//            _this->rx_thrd_res = QtConcurrent::run(rxThrdFunc, _this);
            char data[257];
            while(_this->conn_alive) {
                    {
                        std::lock_guard<std::mutex> lock(_this->mpiMtx);
                        int l = _this->modemPI.receive_rx_data(data, 256);
                        if (l < 0) {
                            if (l == -1) {
                                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FF2222;'>RX ERROR! </span></pre>"));
                                emit _this->do_insertHTMLTRX(QString("<br>"));
                            }
                            if(l == RX_DATA_RET_SW) {
                                emit _this->do_updateActivity(false);
                                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #AAAAAA;'>RX SW(" + QString::number(data[0]) + ") -> </span></pre>"));
                            }
                            if(l == RX_DATA_RET_LEN) {
                                emit _this->do_updateActivity(false);
                                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #AAAAAA;'>LEN(" + QString::number(data[0]) + ") </span></pre>"));
                            }
                            if(l == RX_DATA_RET_ACK) {
                                emit _this->do_updateActivity(false);
                                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #AAAAAA;'>&lt; ACK </span></pre>"));
                                _this->rcvd_pkts++;
                                _this->updateConnStats();
                                emit _this->do_insertHTMLTRX(QString("<br>"));
                            }
                        } else {
                            emit _this->do_updateActivity(false);
                            if(l > RX_DATA_RETSHIFT_DUP) {
                                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FFBBBB;'>DUP </span></pre>"));
                                l -= RX_DATA_RETSHIFT_DUP;
                            }
                            if(l > RX_DATA_RETSHIFT_BADCRC) {
                                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FFBBBB;'>BAD CRC </span></pre>"));
                                l -= RX_DATA_RETSHIFT_BADCRC;
                            }
                            data[l] = '\0';
                            emit _this->do_insertHTMLTRX(QString("<br>"));
                            emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #AAFFAA;'>&lt;&lt;&lt; " + QString(data) + " </span></pre>"));
                            emit _this->do_insertHTMLTRX(QString("<br>"));
                            _this->rcvd_pkts++;
                            _this->updateConnStats();
                        }
                    }
                    usleep(1000);
            }
            _this->param_init = false;
            emit _this->do_updateActivity(true);
            _this->modemPI.stop();
            _this->rx2_thrd_res.waitForFinished();
//            _this->rx_thrd_res.waitForFinished();
            _this->on_updateConStatus(CONN_STATUS_NOTCONNECTED);
        } else {
            _this->on_updateConStatus(CONN_STATUS_NOTCONNECTED);
            _this->gotError("Connection error! Check stderr");
        }
    } else {
        _this->on_updateConStatus(CONN_STATUS_NOTCONNECTED);
    }
}

void MainWindow::rx2ThrdFunc(MainWindow* ctx) {
    MainWindow* _this = (MainWindow*) ctx;
    while(_this->conn_alive) {
//        std::lock_guard<std::mutex> lock(_this->mpiMtx);
        char c;
        int r = _this->modemPI.read_rx2_char(c);
        if(r == 0 && c != '\r') {
//            _this->ui->textBrowser_3->append(QString(c));
            emit _this->do_insertPlainText(QString(c));
        }
    }
}

void MainWindow::txThrdFunc(MainWindow* ctx) {
    MainWindow* _this = (MainWindow*) ctx;
    std::string buffer = _this->ui->lineEdit->text().toStdString();
    int attempts;
    char data[257];
    emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FFFFAA;'>&gt;&gt;&gt; " + QString::fromStdString(buffer) + " </span></pre>"));
    for(attempts = 3; attempts > 0 && _this->conn_alive; attempts--) {
        std::lock_guard<std::mutex> lock(_this->mpiMtx);
        emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FF0000;'>| " + QString::number(attempts) + " -> </span></pre>"));
        emit _this->do_updateActivity(true);
        _this->modemPI.put_tx_data((char*) buffer.c_str(), buffer.length());
        emit _this->do_updateActivity(true);
        _this->modemPI.start_tx_wait();
        bool started = false;
        bool waiting = true;
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        while((waiting || started) && _this->conn_alive) {

            int l = _this->modemPI.receive_rx_data(data, 256);
            if (l < 0) {
                if (l == -1) {
                    emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FF2222;'>RX ERROR! </span></pre>"));
                    break;
                }
                if(l == RX_DATA_RET_SW) {
                    emit _this->do_updateActivity(false);
                    emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #AAAAAA;'>RX SW(" + QString::number(data[0]) + ") -> </span></pre>"));
                    started = true;
                }
                if(l == RX_DATA_RET_LEN) {
                    emit _this->do_updateActivity(false);
                    emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #AAAAAA;'>LEN(" + QString::number(data[0]) + ") -> </span></pre>"));
                }
                if(l == RX_DATA_RET_ACK) {
                    emit _this->do_updateActivity(false);
                    emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #AAFFAA;'>ACK </span></pre>"));
                    attempts = 0;
                    break;
                }
            } else {
                emit _this->do_updateActivity(false);
                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FFAAAA;'>NOT ACK </span></pre>"));
                started = false;
                waiting = false;
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            uint32_t required_time = (1000000UL / (_this->ui->spinBox->value())) * 128;
            if(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() >= required_time && !started) {
                emit _this->do_insertHTMLTRX(QString("<pre><span style='color: #FFAAAA;'>NACK </span></pre>"));
                waiting = false;
            }
        }
    }
    _this->sent_pkts++;
    _this->updateConnStats();
    emit _this->do_insertHTMLTRX(QString("<br>"));
    emit _this->do_sendingFinished();
}

void MainWindow::refreshActivity() {
    statusbar_activity.setText(QString("<pre><span style='color: #FF0000;'>") + QString(act_tx ? "↑" : " ") + QString("</span><span style='color: #00FF00;'>") + QString(act_rx ? "↓" : " ") + QString("</span></pre>"));
}

void MainWindow::reloadParams() {
    param_init = false;
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(std::size(paramsList));
    ui->tableWidget->setColumnCount(2);
    for (int row = 0; row < std::size(paramsList); ++row) {
        QTableWidgetItem *itemA = new QTableWidgetItem(paramsList[row].displayName);
        QTableWidgetItem *itemB;
        uint8_t buff[5];
        QBrush brush;
        QColor brushColor;
        usleep(5000);
        emit do_updateActivity(false);
        int r = modemPI.read_param(std::string(paramsList[row].name), &buff, 5);
        if(r <= 0) {
            if(paramsList[row].format == PARAM_DEF_TYPE_INT) {
                itemB = new QTableWidgetItem(QString::number((int)paramsList[row].def_val));
            } else if(paramsList[row].format == PARAM_DEF_TYPE_FLOAT) {
                itemB = new QTableWidgetItem(QString::number((float)paramsList[row].def_val, 'f'));
            } else {
                itemB = new QTableWidgetItem(QStringLiteral("0x%1").arg((uint8_t)paramsList[row].def_val, 2, 16, QLatin1Char('0')));
            }
            brushColor.setRgb(255 ,200, 200);
        } else {
            if(paramsList[row].format == PARAM_DEF_TYPE_INT) {
                itemB = new QTableWidgetItem(QString::number(*((uint32_t*)buff)));
            } else if(paramsList[row].format == PARAM_DEF_TYPE_FLOAT) {
                float x = *((float*)buff);
                itemB = new QTableWidgetItem(QString::number(x, 'f'));
            } else {
                itemB = new QTableWidgetItem(QStringLiteral("0x%1").arg(*((uint8_t*)buff), 2, 16, QLatin1Char('0')));
            }
            brushColor.setRgb(200 ,255, 200);
        }
        brush.setColor(brushColor);
        brush.setStyle(Qt::SolidPattern);
        itemB->setForeground(brush);
        itemA->setFlags(itemA->flags() ^ Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 0, itemA);
        ui->tableWidget->setItem(row, 1, itemB);
    }
    usleep(1000);
    param_init = true;
}

void MainWindow::reloadModemCfg() {
    std::lock_guard<std::mutex> lock(mpiMtx);
    cbmodem::pc_packet_interface::modes newmode;
    switch(ui->comboBox_2->currentIndex()) {
        case 0:
            newmode = cbmodem::pc_packet_interface::PC_PI_MODE_NORMAL_BFSK;
            break;
        case 1:
            newmode = cbmodem::pc_packet_interface::PC_PI_MODE_NORMAL_MFSK;
            break;
        case 2:
            newmode = cbmodem::pc_packet_interface::PC_PI_MODE_NORMAL_MSK;
            break;
        default:
            newmode = cbmodem::pc_packet_interface::PC_PI_MODE_SDR;
            break;
    }
    emit do_updateActivity(true);
    modemPI.stop_rx();
    usleep(1000);
    emit do_updateActivity(true);
    modemPI.change_mode(newmode, false);
    usleep(1000);
    emit do_updateActivity(true);
    modemPI.set_speed((float)ui->spinBox->value(), (float)ui->spinBox_2->value(), ui->spinBox_3->value());
    usleep(1000);
    emit do_updateActivity(true);
    modemPI.start_rx();
    usleep(1000);
}

void MainWindow::updateConnStats() {
    uint32_t mins = conn_time / 60UL;
    uint32_t secs = conn_time % 60UL;
    ui->label_2->setText(QStringLiteral("%1").arg(mins, 2, 10, QLatin1Char('0')) + ":" + QStringLiteral("%1").arg(secs, 2, 10, QLatin1Char('0')));
    ui->label_6->setText(QString::number(rcvd_pkts));
    ui->label_4->setText(QString::number(sent_pkts));
}

void MainWindow::on_updateConStatus(CONN_STATUS s) {
    curr_status = s;
    if(s == CONN_STATUS_NOTCONNECTED) {
        ui->pushButton->setText("Connect");
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->spinBox->setEnabled(false);
        ui->spinBox_2->setEnabled(false);
        ui->spinBox_3->setEnabled(false);
        ui->spinBox_4->setEnabled(false);
        ui->comboBox_2->setEnabled(false);
        ui->tableWidget->setEnabled(false);
        ui->pushButton_7->setEnabled(false);
        ui->lineEdit->setEnabled(false);
        ui->pushButton_3->setEnabled(false);

        ui->statusbar->clearMessage();
        act_tx = false;
        act_rx = false;
        statusbar_conn.setText("Disconnected");
        refreshActivity();
    } else if(s == CONN_STATUS_CONNECTING) {
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->comboBox->setEnabled(false);

        ui->statusbar->clearMessage();
        statusbar_conn.setText("Connecting...");
    } else {
        ui->pushButton->setText("Disconnect");
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->spinBox->setEnabled(true);
        ui->spinBox_2->setEnabled(true);
        ui->spinBox_3->setEnabled(true);
        ui->spinBox_4->setEnabled(true);
        ui->comboBox_2->setEnabled(true);
        ui->tableWidget->setEnabled(true);
        ui->pushButton_7->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->pushButton_3->setEnabled(true);

        ui->statusbar->clearMessage();
        statusbar_conn.setText("Connected");
    }
}

void MainWindow::on_actrxtimer_timeout() {
    act_rx = false;
    refreshActivity();
}

void MainWindow::on_acttxtimer_timeout() {
    act_tx = false;
    refreshActivity();
}

void MainWindow::on_conntimetimer_timeout() {
    if(conn_alive) {
        conn_time++;
        updateConnStats();
    }
}

void MainWindow::on_do_updateActivity(bool tx) {
    if(tx) {
        act_tx = true;
        act_tx_tmr.start(200);
    } else {
        act_rx = true;
        act_rx_tmr.start(200);
    }
    refreshActivity();
}

void MainWindow::gotError(QString err) {
    mb.critical(this, "ERROR!", "Error!\n" + err);
}

void MainWindow::on_do_insertPlainText(const QString &text) {
    bool x = (ui->textBrowser_3->verticalScrollBar()->maximum() - ui->textBrowser_3->verticalScrollBar()->value()) <= 10;
    ui->textBrowser_3->insertPlainText(text);
    if(x) {
        ui->textBrowser_3->verticalScrollBar()->setValue(ui->textBrowser_3->verticalScrollBar()->maximum());
    }
}

void MainWindow::on_do_insertHTMLTRX(const QString &text) {
    bool x = (ui->textBrowser->verticalScrollBar()->maximum() - ui->textBrowser->verticalScrollBar()->value()) <= 10;
    ui->textBrowser->insertHtml(text);
    if(x) {
        ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
    }
}

void MainWindow::on_sending_finished() {
    if(conn_alive) {
        ui->lineEdit->setEnabled(true);
        ui->pushButton_3->setEnabled(true);
        ui->lineEdit->clear();
        ui->lineEdit->setFocus();
    }
}

void MainWindow::on_pushButton_2_clicked() {
    //Refresh
    QList<QSerialPortInfo> aports = QSerialPortInfo::availablePorts();
    ui->comboBox->clear();
    QString prevselected = settings.value("prevport").toString();
    for(int i = 0; i < aports.size(); i++) {
        ui->comboBox->addItem("/dev/" + aports[i].portName());
    }
    for(int i = 0; i < aports.size(); i++) {
        if(aports[i].portName() == prevselected) {
            ui->comboBox->setCurrentIndex(i);
        }
    }
}


void MainWindow::on_pushButton_clicked() {
    if(curr_status == CONN_STATUS_NOTCONNECTED) {
        QString port_name = ui->comboBox->currentText();
        settings.setValue("prevport", port_name);
        if(!conn_alive) {
            modemPI.init(port_name.toStdString());
            conn_alive = true;
            ui->textBrowser->clear();
            conn_thrd_res = QtConcurrent::run(connThrdFunc, this);
        }
    } else {
        conn_alive = false;
        param_init = false;
    }
}


void MainWindow::on_tableWidget_cellActivated(int row, int column) {}

void MainWindow::on_tableWidget_cellChanged(int row, int column) {
    if(param_set_cnt > 0) {
        param_set_cnt--;
        return;
    }
    if(conn_alive && param_init) {
        if(column == 1) {
            QTableWidgetItem* newitem = ui->tableWidget->item(row, column);
            if(newitem != nullptr) {
                ui->tableWidget->setEnabled(false);
                QTableWidgetItem *itemB;
                uint8_t buff[5];
                QBrush brush;
                QColor brushColor;
                QString newData = newitem->data(Qt::DisplayRole).toString();
                if(paramsList[row].format == PARAM_DEF_TYPE_INT) {
                    bool ok;
                    uint32_t newVal = newData.toInt(&ok);
                    if (ok && newVal > paramsList[row].min_val && newVal < paramsList[row].max_val) {
                        modemPI.write_param(paramsList[row].name, (uint8_t*) &newVal, 4);
                    }
                    usleep(5000);
                    int r = modemPI.read_param(std::string(paramsList[row].name), &buff, 5);
                    if(r <= 0) {
                        itemB = new QTableWidgetItem(QString::number((int)paramsList[row].def_val));
                        brushColor.setRgb(255 ,200, 200);
                    } else {
                        itemB = new QTableWidgetItem(QString::number(newVal));
                        brushColor.setRgb(255 ,255, 200);
                    }
                } else if(paramsList[row].format == PARAM_DEF_TYPE_FLOAT) {
                    bool ok;
                    float newVal = newData.toFloat(&ok);
                    if (ok && newVal > paramsList[row].min_val && newVal < paramsList[row].max_val) {
                        modemPI.write_param(paramsList[row].name, (uint8_t*) &newVal, 4);
                    }
                    usleep(5000);
                    int r = modemPI.read_param(std::string(paramsList[row].name), &buff, 5);
                    if(r <= 0) {
                        itemB = new QTableWidgetItem(QString::number((float)paramsList[row].def_val, 'f'));
                        brushColor.setRgb(255 ,200, 200);
                    } else {
                        itemB = new QTableWidgetItem(QString::number(newVal, 'f'));
                        brushColor.setRgb(255 ,255, 200);
                    }
                } else {
                    bool ok;
                    uint32_t newVal = newData.remove(0, 2).toUInt(&ok, 16);
                    if (ok && newVal > paramsList[row].min_val && newVal < paramsList[row].max_val) {
                        modemPI.write_param(paramsList[row].name, (uint8_t*) &newVal, 4);
                    }
                    usleep(5000);
                    int r = modemPI.read_param(std::string(paramsList[row].name), &buff, 5);
                    if(r <= 0) {
                        itemB = new QTableWidgetItem(QStringLiteral("0x%1").arg((uint8_t)paramsList[row].def_val, 2, 16, QLatin1Char('0')));
                        brushColor.setRgb(255 ,200, 200);
                    } else {
                        itemB = new QTableWidgetItem(QStringLiteral("0x%1").arg(newVal, 2, 16, QLatin1Char('0')));
                        brushColor.setRgb(255 ,255, 200);
                    }
                }
                brush.setColor(brushColor);
                brush.setStyle(Qt::SolidPattern);
                itemB->setForeground(brush);
                param_set_cnt++;
                ui->tableWidget->setItem(row, 1, itemB);
                ui->tableWidget->setEnabled(true);
            }
        }
    }
}


void MainWindow::on_pushButton_7_clicked() {
    if(conn_alive && param_init) {
        ui->tableWidget->setEnabled(false);
        modemPI.store_params();
        reloadParams();
        ui->tableWidget->setEnabled(true);
    }
}


void MainWindow::on_pushButton_6_clicked() {
    ui->textBrowser_3->clear();
}


void MainWindow::on_pushButton_5_clicked() {
    ui->textBrowser->clear();
}


void MainWindow::on_spinBox_4_valueChanged(int arg1){
    if(conn_alive && param_init) {
        modemPI.set_fr(arg1);
        settings.setValue("param_freq", arg1);
    }
}


void MainWindow::on_comboBox_2_currentIndexChanged(int index) {
    if(conn_alive && param_init) {
        reloadModemCfg();
        settings.setValue("param_mode", index);
    }
}


void MainWindow::on_spinBox_valueChanged(int arg1) {
    if(conn_alive && param_init) {
        reloadModemCfg();
        settings.setValue("param_speed", arg1);
    }
}


void MainWindow::on_spinBox_2_valueChanged(int arg1) {
    if(conn_alive && param_init) {
        reloadModemCfg();
        settings.setValue("param_frspacing", arg1);
    }
}


void MainWindow::on_spinBox_3_valueChanged(int arg1) {
    if(conn_alive && param_init && (arg1 & (arg1 - 1)) == 0) {
        reloadModemCfg();
        settings.setValue("param_frcnt", arg1);
    }
}


void MainWindow::on_pushButton_3_clicked() {
    if(conn_alive && param_init) {
        std::string buffer = ui->lineEdit->text().toStdString();
        if(buffer.length() <= 63) {
            ui->lineEdit->setEnabled(false);
            ui->pushButton_3->setEnabled(false);
            QtConcurrent::run(txThrdFunc, this);
        }
    }
}

