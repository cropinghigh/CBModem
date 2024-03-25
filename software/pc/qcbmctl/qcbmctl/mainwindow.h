#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include <qtconcurrentrun.h>
#include <qfuture.h>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QThreadPool>

#include <libcbm.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum CONN_STATUS {
    CONN_STATUS_NOTCONNECTED,
    CONN_STATUS_CONNECTING,
    CONN_STATUS_CONNECTED
};

enum param_def_type {
    PARAM_DEF_TYPE_INT,
    PARAM_DEF_TYPE_FLOAT,
    PARAM_DEF_TYPE_HEX,
};

struct param_def {
    const char* name;
    const char* displayName;
    const param_def_type format;
    const float def_val;
    const float min_val;
    const float max_val;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void connThrdFunc(MainWindow* ctx);
    static void rx2ThrdFunc(MainWindow* ctx);
    static void txThrdFunc(MainWindow* ctx);
//    static void rxThrdFunc(MainWindow* ctx);

    void refreshActivity();
    void reloadParams();
    void reloadModemCfg();
    void updateConnStats();

private slots:
    void on_updateConStatus(CONN_STATUS s);
    void on_do_updateActivity(bool tx);
    void on_actrxtimer_timeout();
    void on_acttxtimer_timeout();
    void on_conntimetimer_timeout();
    void gotError(QString err);
    void on_do_insertPlainText(const QString &text);
    void on_do_insertHTMLTRX(const QString &text);
    void on_sending_finished();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_tableWidget_cellActivated(int row, int column);

    void on_tableWidget_cellChanged(int row, int column);

    void on_pushButton_7_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_5_clicked();

    void on_spinBox_4_valueChanged(int arg1);

    void on_comboBox_2_currentIndexChanged(int index);

    void on_spinBox_valueChanged(int arg1);

    void on_spinBox_2_valueChanged(int arg1);

    void on_spinBox_3_valueChanged(int arg1);

    void on_pushButton_3_clicked();

signals:
    void do_insertPlainText(const QString &text);
    void do_insertHTMLTRX(const QString &text);
    void do_sendingFinished();
    void do_updateActivity(bool tx);

private:
    Ui::MainWindow *ui;
    QSettings settings;

    cbmodem::ModemPacketInterface modemPI;
    bool conn_alive = false;
    bool param_init = false;
    std::mutex mpiMtx;
    int param_set_cnt = 0;
    QFuture<void> conn_thrd_res;
    QFuture<void> rx2_thrd_res;
//    QFuture<void> rx_thrd_res;

    CONN_STATUS curr_status = CONN_STATUS_NOTCONNECTED;

    QMessageBox mb;
    QLabel statusbar_conn;
    QLabel statusbar_activity;
    bool act_tx = false;
    bool act_rx = false;
    QTimer act_tx_tmr;
    QTimer act_rx_tmr;
    QTimer conn_time_tmr;
    uint64_t conn_time = 0;
    int sent_pkts = 0;
    int rcvd_pkts = 0;
    static constexpr const param_def paramsList[] = {
        {"dm_rift", "Receiver input FIR taps", PARAM_DEF_TYPE_INT, 33, 5, 1025},
        {"dm_risr", "Receiver input SR", PARAM_DEF_TYPE_INT, 100000, 1000, 1000000},
        {"dm_rdcb", "Receiver DC block rate", PARAM_DEF_TYPE_FLOAT, 0.9, 0, 3},
        {"dm_ragc", "Receiver AGC rate", PARAM_DEF_TYPE_FLOAT, 0.6, 0, 3},
        {"dm_rnsr", "Normal RX SR", PARAM_DEF_TYPE_INT, 2000, 100, 100000},
        {"dm_tisr", "Tx input SR", PARAM_DEF_TYPE_INT, 10000, 100, 100000},
        {"dm_tbfkt", "BFSK TX taps", PARAM_DEF_TYPE_INT, 33, 5, 1025},
        {"dm_tmfkt", "MFSK TX taps", PARAM_DEF_TYPE_INT, 33, 5, 1025},
        {"dm_i2cl", "Low I2C spd, k", PARAM_DEF_TYPE_INT, 400, 10, 1000},
        {"dm_i2ch", "High I2C spd, k", PARAM_DEF_TYPE_INT, 900, 10, 1000},
        {"dm_siaddr", "Si5351 I2C addr", PARAM_DEF_TYPE_HEX, 0x60, 0x00, 0xff},
        {"dm_sixf", "Si5351 XTAL FR", PARAM_DEF_TYPE_FLOAT, 25000000, 1000000, 40000000},
        {"dm_adcbw", "ADC bitwidth", PARAM_DEF_TYPE_INT, 12, 8, 12},
        {"dm_tca", "TX comp. coeff A", PARAM_DEF_TYPE_INT, 6, 0, 1024},
        {"dm_tcb", "TX comp. coeff B", PARAM_DEF_TYPE_INT, 0, 0, 1024},
        {"dm_tdsr", "TX DAC SR", PARAM_DEF_TYPE_INT, 100000, 1000, 1000000},
        {"dm_toft", "TX filt taps", PARAM_DEF_TYPE_INT, 33, 5, 1025},
        {"dm_txspls", "Spls/tx op.", PARAM_DEF_TYPE_INT, 128, 5, 1025},
        {"dm_rxspls", "Spls/rx op.", PARAM_DEF_TYPE_INT, 4, 5, 1025},
        {"dm_sdrrxspls", "Spls/sdr op.", PARAM_DEF_TYPE_INT, 16, 5, 1025},
        {"dm_loopbw", "Timing recovery loop bw", PARAM_DEF_TYPE_FLOAT, 0.0005, 0, 10},
    };
    QFont font;
};
#endif // MAINWINDOW_H
