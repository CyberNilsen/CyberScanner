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
#include <QUdpSocket>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QElapsedTimer>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QThreadPool>
#include <QRunnable>
#include <QMutexLocker>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Enums for scan types and timing
enum class ScanType {
    TCP_CONNECT,
    TCP_SYN,
    UDP_SCAN,
    TCP_FIN,
    TCP_XMAS,
    TCP_NULL,
    TCP_ACK,
    TCP_WINDOW
};

enum class TimingTemplate {
    T0_PARANOID,
    T1_SNEAKY,
    T2_POLITE,
    T3_NORMAL,
    T4_AGGRESSIVE,
    T5_INSANE
};

// Forward declarations
class PortScanner;
class PortScanTask;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Help menu slots
    void on_actionAbout_triggered();
    void on_actionGithub_triggered();

    // Combo box slots for presets
    void on_comboBox_presets_currentTextChanged(const QString &text);
    void on_comboBox_portPresets_currentTextChanged(const QString &text);

    // New slots for scan options
    void on_comboBox_scanType_currentTextChanged(const QString &text);
    void on_comboBox_timing_currentTextChanged(const QString &text);
    void on_checkBox_osDetection_toggled(bool checked);
    void on_checkBox_aggressiveScan_toggled(bool checked);
    void on_checkBox_detectService_toggled(bool checked);

    // Button slots
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_clearLog_clicked();
    void on_pushButton_saveLog_clicked();

    // Filter slot
    void on_lineEdit_filter_textChanged(const QString &text);
    void on_comboBox_filterType_currentTextChanged(const QString &text);

    // Scanner slots
    void onScanStarted();
    void onScanFinished();
    void onScanProgress(int current, int total);
    void onPortResult(int port, const QString &status, const QString &service, const QString &banner, int responseTime);
    void onScanError(const QString &error);
    void onLogMessage(const QString &message);
    void onOSDetectionResult(const QString &osInfo);

private:
    Ui::MainWindow *ui;
    PortScanner *scanner;
    QTimer *updateTimer;
    QElapsedTimer scanTimer;
    int totalPorts;
    int scannedPorts;
    int openPorts;

    // Scan configuration
    ScanType currentScanType;
    TimingTemplate currentTiming;
    bool serviceDetectionEnabled;
    bool osDetectionEnabled;
    bool aggressiveScanEnabled;

    // Helper functions
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

    // New helper functions
    ScanType getScanTypeFromCombo();
    TimingTemplate getTimingFromCombo();

    // Logging
    void addLogMessage(const QString &message);


};

// Enhanced Port Scanner Class
class PortScanner : public QObject
{
    Q_OBJECT
public:
    explicit PortScanner(QObject *parent = nullptr);
    ~PortScanner();

    void startScan(const QString &target, const QList<int> &ports, ScanType scanType,
                   TimingTemplate timing, bool serviceDetection, bool osDetection, bool aggressive);
    void stopScan();
    bool isScanning() const;

public slots:
    void portScanned(int port, const QString &status, const QString &service,
                     const QString &banner, int responseTime);

signals:
    void scanStarted();
    void scanFinished();
    void scanProgress(int current, int total);
    void portResult(int port, const QString &status, const QString &service, const QString &banner, int responseTime);
    void scanError(const QString &error);
    void logMessage(const QString &message);
    void osDetectionResult(const QString &osInfo);

private slots:
    void onNmapFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString targetHost;
    QList<int> portList;
    bool scanning;
    int connectionTimeout;
    int completedScans;

    // Enhanced scan options
    ScanType scanType;
    TimingTemplate timingTemplate;
    bool enableServiceDetection;
    bool enableOSDetection;
    bool enableAggressiveScan;

    QProcess *nmapProcess;

    // Helper methods
    void performOSDetection(const QString &target);
    void performServiceDetection(const QString &target, const QList<int> &openPorts);
    QString buildNmapCommand(const QString &target, const QList<int> &ports);
    int getTimeoutFromTiming(TimingTemplate timing); // Added this declaration

    int getOptimalThreadCount(TimingTemplate timing, ScanType scanType);
    QString getScanTypeName(ScanType scanType);
    bool tryNmapOSDetection(const QString &target);
    void performSimpleOSDetection(const QString &target);
};

#endif // MAINWINDOW_H
