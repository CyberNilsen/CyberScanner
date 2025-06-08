#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QElapsedTimer>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class PortScanner;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_actionAbout_triggered();
    void on_actionGithub_triggered();

    void on_comboBox_presets_currentTextChanged(const QString &text);
    void on_comboBox_portPresets_currentTextChanged(const QString &text);

    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_clearLog_clicked();
    void on_pushButton_saveLog_clicked();

    void on_lineEdit_filter_textChanged(const QString &text);
    void on_comboBox_filterType_currentTextChanged(const QString &text);

    void onScanStarted();
    void onScanFinished();
    void onScanProgress(int current, int total);
    void onPortResult(int port, const QString &status, const QString &service, const QString &banner, int responseTime);
    void onScanError(const QString &error);
    void onLogMessage(const QString &message);

private:
    Ui::MainWindow *ui;
    PortScanner *scanner;
    QTimer *updateTimer;
    QElapsedTimer scanTimer;
    int totalPorts;
    int scannedPorts;
    int openPorts;

    void showAbout();
    void openGithub();
    void addSampleResults();
    void updateUI();
    void clearResults();
    void applyFilters();

    // Preset handling functions
    void applyTargetPreset(const QString &preset);
    void applyPortPreset(const QString &preset);

    // Port parsing functions
    QList<int> parsePortRange(const QString &portString);
    bool isValidTarget(const QString &target);

    // Logging
    void addLogMessage(const QString &message);
};

// Port Scanner Worker Class
class PortScanner : public QObject
{
    Q_OBJECT

public:
    explicit PortScanner(QObject *parent = nullptr);
    ~PortScanner();

    void startScan(const QString &target, const QList<int> &ports);
    void stopScan();
    bool isScanning() const { return scanning; }

signals:
    void scanStarted();
    void scanFinished();
    void scanProgress(int current, int total);
    void portResult(int port, const QString &status, const QString &service, const QString &banner, int responseTime);
    void scanError(const QString &error);
    void logMessage(const QString &message);

private slots:
    void scanNextPort();
    void onSocketConnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketTimeout();

private:
    QString targetHost;
    QList<int> portList;
    int currentPortIndex;
    QTcpSocket *socket;
    QTimer *timeoutTimer;
    QElapsedTimer portTimer;
    bool scanning;
    int connectionTimeout;

    QString getServiceName(int port);
    QString grabBanner(QTcpSocket *socket, int port);
    void connectToNextPort();
};

#endif // MAINWINDOW_H
