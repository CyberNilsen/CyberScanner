#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThreadPool>
#include <QRunnable>
#include <QMutexLocker>
#include <algorithm>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>


class PortScanTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    PortScanTask(const QString &host, int port, int timeout, ScanType scanType, PortScanner *scanner)
        : host(host), port(port), timeout(timeout), scanType(scanType), scanner(scanner)
    {
        setAutoDelete(true);
    }

    void run() override
    {
        QElapsedTimer timer;
        timer.start();

        QString status;
        QString banner;
        QString service = getServiceName(port);
        int responseTime = 0;

        switch (scanType) {
        case ScanType::TCP_CONNECT:
            performTCPConnectScan(status, banner, responseTime, timer);
            break;
        case ScanType::UDP_SCAN:
            performUDPScan(status, banner, responseTime, timer);
            break;
        case ScanType::TCP_SYN:
            performTCPSynScan(status, banner, responseTime, timer);
            break;
        case ScanType::TCP_FIN:
            performTCPFinScan(status, banner, responseTime, timer);
            break;
        case ScanType::TCP_XMAS:
            performTCPXmasScan(status, banner, responseTime, timer);
            break;
        case ScanType::TCP_NULL:
            performTCPNullScan(status, banner, responseTime, timer);
            break;
        case ScanType::TCP_ACK:
            performTCPAckScan(status, banner, responseTime, timer);
            break;
        case ScanType::TCP_WINDOW:
            performTCPWindowScan(status, banner, responseTime, timer);
            break;
        }

        QMetaObject::invokeMethod(scanner, "portScanned", Qt::QueuedConnection,
                                  Q_ARG(int, port),
                                  Q_ARG(QString, status),
                                  Q_ARG(QString, service),
                                  Q_ARG(QString, banner),
                                  Q_ARG(int, responseTime));
    }

private:
    QString host;
    int port;
    int timeout;
    ScanType scanType;
    PortScanner *scanner;

    void performTCPConnectScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        QTcpSocket socket;
        socket.connectToHost(host, port);

        bool connected = socket.waitForConnected(timeout);
        responseTime = timer.elapsed();

        if (connected) {
            status = "Open";
            banner = grabBanner(&socket, port);
            socket.disconnectFromHost();
        } else {
            QAbstractSocket::SocketError error = socket.error();
            switch (error) {
            case QAbstractSocket::ConnectionRefusedError:
                status = "Closed";
                break;
            case QAbstractSocket::SocketTimeoutError:
                status = "Filtered";
                break;
            case QAbstractSocket::NetworkError:
            case QAbstractSocket::HostNotFoundError:
                status = "Filtered";
                break;
            default:
                status = responseTime < (timeout / 2) ? "Closed" : "Filtered";
                break;
            }
        }
    }

    void performUDPScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        QUdpSocket socket;

        if (!socket.bind()) {
            status = "Error";
            responseTime = timer.elapsed();
            return;
        }

        QByteArray probe = getUDPProbe(port);
        qint64 sent = socket.writeDatagram(probe, QHostAddress(host), port);

        if (sent == -1) {
            status = "Error";
            responseTime = timer.elapsed();
            return;
        }

        bool hasResponse = socket.waitForReadyRead(timeout);
        responseTime = timer.elapsed();

        if (hasResponse) {
            status = "Open";
            QByteArray response;
            QHostAddress sender;
            quint16 senderPort;

            while (socket.hasPendingDatagrams()) {
                response.resize(socket.pendingDatagramSize());
                socket.readDatagram(response.data(), response.size(), &sender, &senderPort);
            }

            banner = QString::fromUtf8(response).trimmed();
            if (banner.length() > 100) {
                banner = banner.left(100) + "...";
            }
        } else {

            status = "Open|Filtered";
        }
    }

    void performTCPSynScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        QTcpSocket socket;
        socket.connectToHost(host, port);

        bool connected = socket.waitForConnected(timeout / 2);
        responseTime = timer.elapsed();

        if (connected) {
            status = "Open";
            socket.disconnectFromHost();
            banner = "";
        } else {
            QAbstractSocket::SocketError error = socket.error();
            switch (error) {
            case QAbstractSocket::ConnectionRefusedError:
                status = "Closed";
                break;
            case QAbstractSocket::SocketTimeoutError:
                status = "Filtered";
                break;
            default:
                status = "Filtered";
                break;
            }
        }
    }

    void performTCPFinScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        status = simulateStealthScan("FIN", responseTime, timer);
        banner = "";
    }

    void performTCPXmasScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        status = simulateStealthScan("XMAS", responseTime, timer);
        banner = "";
    }

    void performTCPNullScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        status = simulateStealthScan("NULL", responseTime, timer);
        banner = "";
    }

    void performTCPAckScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        status = simulateFirewallScan("ACK", responseTime, timer);
        banner = "";
    }

    void performTCPWindowScan(QString &status, QString &banner, int &responseTime, QElapsedTimer &timer)
    {
        status = simulateStealthScan("Window", responseTime, timer);
        banner = "";
    }

    QString simulateStealthScan(const QString &scanType, int &responseTime, QElapsedTimer &timer)
    {

        QTcpSocket socket;
        socket.connectToHost(host, port);

        bool result = socket.waitForConnected(timeout / 3);
        responseTime = timer.elapsed();

        if (result) {
            socket.disconnectFromHost();

            return "Open";
        } else {
            QAbstractSocket::SocketError error = socket.error();
            if (error == QAbstractSocket::ConnectionRefusedError) {
                return "Closed";
            } else {
                return "Filtered";
            }
        }
    }

    QString simulateFirewallScan(const QString &scanType, int &responseTime, QElapsedTimer &timer)
    {
        QTcpSocket socket;
        socket.connectToHost(host, port);

        bool result = socket.waitForConnected(timeout / 4);
        responseTime = timer.elapsed();

        if (result) {
            socket.disconnectFromHost();
            return "Unfiltered";
        } else {
            return "Filtered";
        }
    }

    QByteArray getUDPProbe(int port)
    {
        switch (port) {
        case 53:
            return QByteArray::fromHex("1234010000010000000000000377777706676F6F676C6503636F6D0000010001");
        case 67:
            return QByteArray::fromHex("0101060000003d1d00000000000000000000000000000000");
        case 69:
            return QByteArray("\x00\x01test\x00octet\x00", 12);
        case 123:
            return QByteArray::fromHex("1B0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
        case 161:
            return QByteArray::fromHex("302902010004067075626C6963A01C02020000020100020100308E");
        case 514:
            return QByteArray("<14>Test message");
        default:
            return QByteArray("UDP_PROBE_" + QByteArray::number(port));
        }
    }

    QString getServiceName(int port)
    {
        static QHash<int, QString> services;
        if (services.isEmpty()) {

            services[21] = "FTP";
            services[22] = "SSH";
            services[23] = "Telnet";
            services[25] = "SMTP";
            services[53] = "DNS";
            services[80] = "HTTP";
            services[110] = "POP3";
            services[143] = "IMAP";
            services[443] = "HTTPS";
            services[993] = "IMAPS";
            services[995] = "POP3S";
            services[3389] = "RDP";
            services[8080] = "HTTP-Alt";
            services[8443] = "HTTPS-Alt";
            services[135] = "RPC";
            services[139] = "NetBIOS";
            services[445] = "SMB";
            services[1433] = "MSSQL";
            services[3306] = "MySQL";
            services[5432] = "PostgreSQL";
            services[6379] = "Redis";
            services[27017] = "MongoDB";
            services[1521] = "Oracle";
            services[5060] = "SIP";
            services[5061] = "SIP-TLS";

            services[67] = "DHCP";
            services[68] = "DHCP";
            services[69] = "TFTP";
            services[123] = "NTP";
            services[161] = "SNMP";
            services[162] = "SNMP-Trap";
            services[514] = "Syslog";
            services[520] = "RIP";
            services[1900] = "UPnP";
        }
        return services.value(port, "Unknown");
    }

    QString grabBanner(QTcpSocket *socket, int port)
    {
        if (!socket || !socket->isOpen()) {
            return "";
        }

        QString banner;

        switch (port) {
        case 21:
            if (socket->waitForReadyRead(1000)) {
                banner = QString::fromUtf8(socket->readAll()).trimmed();
            }
            break;

        case 22:
            if (socket->waitForReadyRead(1000)) {
                banner = QString::fromUtf8(socket->readAll()).trimmed();
            }
            break;

        case 25:
            if (socket->waitForReadyRead(1000)) {
                banner = QString::fromUtf8(socket->readAll()).trimmed();
            }
            break;

        case 80:
        case 8080:
            socket->write("GET / HTTP/1.0\r\nHost: " + host.toUtf8() + "\r\n\r\n");
            if (socket->waitForReadyRead(2000)) {
                QByteArray data = socket->readAll();
                banner = QString::fromUtf8(data).trimmed();

                QRegularExpression serverRegex("Server: ([^\r\n]+)");
                QRegularExpressionMatch match = serverRegex.match(banner);
                if (match.hasMatch()) {
                    banner = match.captured(1);
                } else {

                    QStringList lines = banner.split('\n');
                    if (!lines.isEmpty()) {
                        banner = lines.first().trimmed();
                    }
                }
            }
            break;

        case 443:
        case 8443:
            banner = "HTTPS/SSL";
            break;

        case 110:
            if (socket->waitForReadyRead(1000)) {
                banner = QString::fromUtf8(socket->readAll()).trimmed();
            }
            break;

        case 143:
            if (socket->waitForReadyRead(1000)) {
                banner = QString::fromUtf8(socket->readAll()).trimmed();
            }
            break;

        case 3389:
            banner = "RDP";
            break;

        default:

            if (socket->waitForReadyRead(500)) {
                banner = QString::fromUtf8(socket->readAll()).trimmed();
            }
            break;
        }

        banner = banner.remove(QRegularExpression("[\\x00-\\x1F\\x7F-\\xFF]"));
        if (banner.length() > 100) {
            banner = banner.left(100) + "...";
        }

        return banner;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scanner(nullptr)
    , totalPorts(0)
    , scannedPorts(0)
    , openPorts(0)
    , currentScanType(ScanType::TCP_CONNECT)
    , currentTiming(TimingTemplate::T3_NORMAL)
    , serviceDetectionEnabled(true)
    , osDetectionEnabled(false)
    , aggressiveScanEnabled(false)
{
    ui->setupUi(this);

    scanner = new PortScanner(this);

    connect(scanner, &PortScanner::scanStarted, this, &MainWindow::onScanStarted);
    connect(scanner, &PortScanner::scanFinished, this, &MainWindow::onScanFinished);
    connect(scanner, &PortScanner::scanProgress, this, &MainWindow::onScanProgress);
    connect(scanner, &PortScanner::portResult, this, &MainWindow::onPortResult);
    connect(scanner, &PortScanner::scanError, this, &MainWindow::onScanError);
    connect(scanner, &PortScanner::logMessage, this, &MainWindow::onLogMessage);
    connect(scanner, &PortScanner::osDetectionResult, this, &MainWindow::onOSDetectionResult);

    ui->tableWidget_results->setColumnCount(6);
    QStringList headers;
    headers << "Port" << "Protocol" << "Status" << "Service" << "Banner" << "Response Time";
    ui->tableWidget_results->setHorizontalHeaderLabels(headers);
    ui->tableWidget_results->resizeColumnsToContents();

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateUI);

    ui->comboBox_timing->setCurrentIndex(3);

    addLogMessage("Enhanced Port Scanner initialized - Ready to scan");
    addLogMessage("Features: Multiple scan types, OS detection, Service detection");
}

MainWindow::~MainWindow()
{
    if (scanner && scanner->isScanning()) {
        scanner->stopScan();
    }
    delete ui;
}

void MainWindow::on_comboBox_scanType_currentTextChanged(const QString &text)
{
    currentScanType = getScanTypeFromCombo();
    addLogMessage(QString("Scan type changed to: %1").arg(text));

    if (currentScanType == ScanType::UDP_SCAN) {
        ui->tableWidget_results->horizontalHeaderItem(1)->setText("Protocol (UDP)");
    } else {
        ui->tableWidget_results->horizontalHeaderItem(1)->setText("Protocol (TCP)");
    }
}

void MainWindow::on_comboBox_timing_currentTextChanged(const QString &text)
{
    currentTiming = getTimingFromCombo();
    addLogMessage(QString("Timing template changed to: %1").arg(text));
}

void MainWindow::on_checkBox_osDetection_toggled(bool checked)
{
    osDetectionEnabled = checked;
    if (checked) {
        addLogMessage("OS Detection enabled (requires admin privileges for best results)");
    } else {
        addLogMessage("OS Detection disabled");
    }
}

void MainWindow::on_checkBox_aggressiveScan_toggled(bool checked)
{
    aggressiveScanEnabled = checked;
    if (checked) {

        ui->checkBox_osDetection->setChecked(true);
        ui->checkBox_detectService->setChecked(true);
        osDetectionEnabled = true;
        serviceDetectionEnabled = true;
        addLogMessage("Aggressive scan enabled (includes OS detection, service detection, and scripts)");
    } else {
        addLogMessage("Aggressive scan disabled");
    }
}

void MainWindow::on_checkBox_detectService_toggled(bool checked)
{
    serviceDetectionEnabled = checked;
    if (checked) {
        addLogMessage("Service Detection enabled");
    } else {
        addLogMessage("Service Detection disabled");
    }
}

void MainWindow::on_pushButton_start_clicked()
{
    QString target = ui->lineEdit_target->text().trimmed();
    if (target.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a target host or IP address.");
        return;
    }

    if (!isValidTarget(target)) {
        QMessageBox::warning(this, "Error", "Invalid target format. Please enter a valid IP address or hostname.");
        return;
    }

    QList<int> ports;
    if (!ui->lineEdit_customPorts->text().trimmed().isEmpty()) {
        ports = parsePortRange(ui->lineEdit_customPorts->text());
    } else {
        int fromPort = ui->spinBox_portFrom->value();
        int toPort = ui->spinBox_portTo->value();
        if (fromPort > toPort) {
            QMessageBox::warning(this, "Error", "Invalid port range. 'From' port must be less than or equal to 'To' port.");
            return;
        }
        for (int i = fromPort; i <= toPort; i++) {
            ports.append(i);
        }
    }

    if (ports.isEmpty()) {
        QMessageBox::warning(this, "Error", "No valid ports to scan.");
        return;
    }

    clearResults();
    totalPorts = ports.size();

    addLogMessage(QString("=== Starting Enhanced Scan ==="));
    addLogMessage(QString("Target: %1").arg(target));
    addLogMessage(QString("Ports: %1 total").arg(totalPorts));
    addLogMessage(QString("Scan Type: %1").arg(ui->comboBox_scanType->currentText()));
    addLogMessage(QString("Timing: %1").arg(ui->comboBox_timing->currentText()));
    addLogMessage(QString("Service Detection: %1").arg(serviceDetectionEnabled ? "Enabled" : "Disabled"));
    addLogMessage(QString("OS Detection: %1").arg(osDetectionEnabled ? "Enabled" : "Disabled"));
    addLogMessage(QString("Aggressive Scan: %1").arg(aggressiveScanEnabled ? "Enabled" : "Disabled"));

    scanner->startScan(target, ports, currentScanType, currentTiming,
                       serviceDetectionEnabled, osDetectionEnabled, aggressiveScanEnabled);
}

ScanType MainWindow::getScanTypeFromCombo()
{
    QString text = ui->comboBox_scanType->currentText();
    if (text.contains("TCP Connect")) return ScanType::TCP_CONNECT;
    if (text.contains("TCP SYN")) return ScanType::TCP_SYN;
    if (text.contains("UDP")) return ScanType::UDP_SCAN;
    if (text.contains("TCP FIN")) return ScanType::TCP_FIN;
    if (text.contains("TCP XMAS")) return ScanType::TCP_XMAS;
    if (text.contains("TCP NULL")) return ScanType::TCP_NULL;
    if (text.contains("TCP ACK")) return ScanType::TCP_ACK;
    if (text.contains("TCP Window")) return ScanType::TCP_WINDOW;
    return ScanType::TCP_CONNECT;
}

TimingTemplate MainWindow::getTimingFromCombo()
{
    QString text = ui->comboBox_timing->currentText();
    if (text.contains("T0")) return TimingTemplate::T0_PARANOID;
    if (text.contains("T1")) return TimingTemplate::T1_SNEAKY;
    if (text.contains("T2")) return TimingTemplate::T2_POLITE;
    if (text.contains("T3")) return TimingTemplate::T3_NORMAL;
    if (text.contains("T4")) return TimingTemplate::T4_AGGRESSIVE;
    if (text.contains("T5")) return TimingTemplate::T5_INSANE;
    return TimingTemplate::T3_NORMAL;
}

void MainWindow::onOSDetectionResult(const QString &osInfo)
{
    addLogMessage(QString("OS Detection Result: %1").arg(osInfo));

    QMessageBox::information(this, "OS Detection",
                             QString("Detected Operating System:\n%1").arg(osInfo));
}

PortScanner::PortScanner(QObject *parent)
    : QObject(parent)
    , scanning(false)
    , connectionTimeout(1000)
    , completedScans(0)
    , scanType(ScanType::TCP_CONNECT)
    , timingTemplate(TimingTemplate::T3_NORMAL)
    , enableServiceDetection(true)
    , enableOSDetection(false)
    , enableAggressiveScan(false)
    , nmapProcess(nullptr)
{
    int optimalThreads = QThread::idealThreadCount() * 4;
    QThreadPool::globalInstance()->setMaxThreadCount(optimalThreads);
}

PortScanner::~PortScanner()
{
    stopScan();
}

void PortScanner::startScan(const QString &target, const QList<int> &ports, ScanType scanType,
                            TimingTemplate timing, bool serviceDetection, bool osDetection, bool aggressive)
{
    if (scanning) return;

    targetHost = target;
    portList = ports;
    this->scanType = scanType;
    this->timingTemplate = timing;
    this->enableServiceDetection = serviceDetection;
    this->enableOSDetection = osDetection;
    this->enableAggressiveScan = aggressive;

    completedScans = 0;
    scanning = true;

    connectionTimeout = getTimeoutFromTiming(timing);

    int threadCount = getOptimalThreadCount(timing, scanType);
    QThreadPool::globalInstance()->setMaxThreadCount(threadCount);

    emit scanStarted();
    emit logMessage(QString("Starting %1 scan with %2 threads, timeout: %3ms")
                        .arg(getScanTypeName(scanType))
                        .arg(threadCount)
                        .arg(connectionTimeout));

    for (int port : ports) {
        PortScanTask *task = new PortScanTask(target, port, connectionTimeout, scanType, this);
        QThreadPool::globalInstance()->start(task);
    }
}

int PortScanner::getOptimalThreadCount(TimingTemplate timing, ScanType scanType)
{
    int baseThreads = QThread::idealThreadCount();

    switch (timing) {
    case TimingTemplate::T0_PARANOID:
        return 1;
    case TimingTemplate::T1_SNEAKY:
        return qMax(1, baseThreads / 4);
    case TimingTemplate::T2_POLITE:
        return qMax(1, baseThreads / 2);
    case TimingTemplate::T3_NORMAL:
        return baseThreads * 2;
    case TimingTemplate::T4_AGGRESSIVE:
        return baseThreads * 4;
    case TimingTemplate::T5_INSANE:
        return baseThreads * 8;
    }

    if (scanType == ScanType::UDP_SCAN) {
        return qMax(1, baseThreads / 2);
    }

    return baseThreads * 2;
}

QString PortScanner::getScanTypeName(ScanType scanType)
{
    switch (scanType) {
    case ScanType::TCP_CONNECT: return "TCP Connect";
    case ScanType::TCP_SYN: return "TCP SYN";
    case ScanType::UDP_SCAN: return "UDP";
    case ScanType::TCP_FIN: return "TCP FIN";
    case ScanType::TCP_XMAS: return "TCP XMAS";
    case ScanType::TCP_NULL: return "TCP NULL";
    case ScanType::TCP_ACK: return "TCP ACK";
    case ScanType::TCP_WINDOW: return "TCP Window";
    }
    return "Unknown";
}

void PortScanner::performOSDetection(const QString &target)
{
    emit logMessage("Performing OS detection...");

    if (tryNmapOSDetection(target)) {
        return;
    }

    performSimpleOSDetection(target);
}

bool PortScanner::tryNmapOSDetection(const QString &target)
{
    if (!nmapProcess) {
        nmapProcess = new QProcess(this);
        connect(nmapProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &PortScanner::onNmapFinished);
    }

    QStringList args;
    args << "-O" << "-v" << target;

    nmapProcess->start("nmap", args);
    return nmapProcess->waitForStarted(1000);
}

void PortScanner::performSimpleOSDetection(const QString &target)
{
    QString osInfo = "OS Detection: ";

    QTcpSocket socket;
    socket.connectToHost(target, 80);

    if (socket.waitForConnected(2000)) {

        osInfo += "Host appears to be running a TCP/IP stack (OS detection requires deeper analysis)";
        socket.disconnectFromHost();
    } else {
        osInfo += "Unable to determine OS - no TCP services responding";
    }

    emit osDetectionResult(osInfo);
}

void PortScanner::stopScan()
{
    if (!scanning) return;

    scanning = false;
    QThreadPool::globalInstance()->clear();
    QThreadPool::globalInstance()->waitForDone(5000);

    if (nmapProcess && nmapProcess->state() != QProcess::NotRunning) {
        nmapProcess->kill();
        nmapProcess->waitForFinished(3000);
    }

    emit scanFinished();
}

void PortScanner::portScanned(int port, const QString &status, const QString &service, const QString &banner, int responseTime)
{
    if (!scanning) return;

    completedScans++;
    emit portResult(port, status, service, banner, responseTime);
    emit scanProgress(completedScans, portList.size());

    if (completedScans >= portList.size()) {

        QList<int> openPorts;

        if (enableOSDetection && !openPorts.isEmpty()) {
            performOSDetection(targetHost);
        }

        if (enableServiceDetection && !openPorts.isEmpty()) {
            performServiceDetection(targetHost, openPorts);
        }

        scanning = false;
        emit scanFinished();
    }
}

int PortScanner::getTimeoutFromTiming(TimingTemplate timing)
{
    switch (timing) {
    case TimingTemplate::T0_PARANOID: return 5000;
    case TimingTemplate::T1_SNEAKY:   return 3000;
    case TimingTemplate::T2_POLITE:   return 2000;
    case TimingTemplate::T3_NORMAL:   return 1000;
    case TimingTemplate::T4_AGGRESSIVE: return 500;
    case TimingTemplate::T5_INSANE:   return 250;
    }
    return 1000;
}


void PortScanner::performServiceDetection(const QString &target, const QList<int> &openPorts)
{
    emit logMessage("Performing enhanced service detection...");
}

void PortScanner::onNmapFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString output = nmapProcess->readAllStandardOutput();
        emit osDetectionResult(output);
    } else {
        emit osDetectionResult("OS detection failed - nmap not available or insufficient privileges");
    }
}

bool PortScanner::isScanning() const
{
    return scanning;
}

void MainWindow::on_pushButton_stop_clicked()
{
    if (scanner && scanner->isScanning()) {
        scanner->stopScan();
        addLogMessage("Scan stopped by user");
    }
}

void MainWindow::on_pushButton_clear_clicked()
{
    clearResults();
    addLogMessage("Results cleared");
}

void MainWindow::on_pushButton_clearLog_clicked()
{
    ui->textEdit_log->clear();
}

void MainWindow::on_pushButton_saveLog_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Log", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->textEdit_log->toPlainText();
            addLogMessage(QString("Log saved to: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, "Error", "Could not save log file.");
        }
    }
}

void MainWindow::on_lineEdit_filter_textChanged(const QString &text)
{
    Q_UNUSED(text)
    applyFilters();
}

void MainWindow::on_comboBox_filterType_currentTextChanged(const QString &text)
{
    Q_UNUSED(text)
    applyFilters();
}

void MainWindow::onScanStarted()
{
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);
    ui->label_status->setText("Status: Scanning...");
    ui->progressBar->setValue(0);
    scannedPorts = 0;
    openPorts = 0;
    scanTimer.start();
    updateTimer->start(250);
}

void MainWindow::onScanFinished()
{
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
    ui->label_status->setText("Status: Completed");
    ui->progressBar->setValue(100);
    updateTimer->stop();

    qint64 elapsed = scanTimer.elapsed();
    QString timeStr = QString("%1:%2.%3").arg(elapsed / 60000, 2, 10, QChar('0'))
                          .arg((elapsed % 60000) / 1000, 2, 10, QChar('0'))
                          .arg((elapsed % 1000) / 10, 2, 10, QChar('0'));
    addLogMessage(QString("Scan completed. Total time: %1").arg(timeStr));
    addLogMessage(QString("Scan rate: %1 ports/second").arg(totalPorts * 1000.0 / elapsed, 0, 'f', 1));
}

void MainWindow::onScanProgress(int current, int total)
{
    scannedPorts = current;
    if (total > 0) {
        int percentage = (current * 100) / total;
        ui->progressBar->setValue(percentage);
    }
}

void MainWindow::onScanError(const QString &error)
{
    addLogMessage(QString("ERROR: %1").arg(error));
    QMessageBox::critical(this, "Scan Error", error);
}

void MainWindow::onLogMessage(const QString &message)
{
    addLogMessage(message);
}

void MainWindow::updateUI()
{
    if (scanner && scanner->isScanning()) {
        qint64 elapsed = scanTimer.elapsed();
        QString timeStr = QString("%1:%2").arg(elapsed / 60000, 2, 10, QChar('0'))
                              .arg((elapsed % 60000) / 1000, 2, 10, QChar('0'));
        ui->label_stats->setText(QString("Scanned: %1 | Open: %2 | Time: %3")
                                     .arg(scannedPorts).arg(openPorts).arg(timeStr));
    }
}

void MainWindow::clearResults()
{
    ui->tableWidget_results->setRowCount(0);
    ui->progressBar->setValue(0);
    ui->label_status->setText("Status: Ready");
    ui->label_stats->setText("Scanned: 0 | Open: 0 | Time: 00:00");
    scannedPorts = 0;
    openPorts = 0;
    totalPorts = 0;
}

void MainWindow::applyFilters()
{
    QString filterText = ui->lineEdit_filter->text().toLower();
    QString filterType = ui->comboBox_filterType->currentText();

    for (int row = 0; row < ui->tableWidget_results->rowCount(); ++row) {
        bool showRow = true;

        if (!filterText.isEmpty()) {
            bool matchFound = false;
            for (int col = 0; col < ui->tableWidget_results->columnCount(); ++col) {
                QTableWidgetItem *item = ui->tableWidget_results->item(row, col);
                if (item && item->text().toLower().contains(filterText)) {
                    matchFound = true;
                    break;
                }
            }
            if (!matchFound) showRow = false;
        }

        if (showRow && filterType != "All") {
            QTableWidgetItem *statusItem = ui->tableWidget_results->item(row, 2);
            if (statusItem) {
                QString status = statusItem->text();
                if (filterType == "Open Only" && status != "Open") {
                    showRow = false;
                } else if (filterType == "Closed Only" && status != "Closed") {
                    showRow = false;
                }
            }
        }

        ui->tableWidget_results->setRowHidden(row, !showRow);
    }
}

void MainWindow::addLogMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);

    ui->textEdit_log->append(logEntry);

    if (ui->checkBox_autoScroll->isChecked()) {
        QTextCursor cursor = ui->textEdit_log->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textEdit_log->setTextCursor(cursor);
    }
}

QList<int> MainWindow::parsePortRange(const QString &portString)
{
    QList<int> ports;
    QStringList parts = portString.split(',', Qt::SkipEmptyParts);

    for (const QString &part : parts) {
        QString trimmed = part.trimmed();
        if (trimmed.contains('-')) {
            QStringList range = trimmed.split('-');
            if (range.size() == 2) {
                bool ok1, ok2;
                int start = range[0].toInt(&ok1);
                int end = range[1].toInt(&ok2);
                if (ok1 && ok2 && start <= end && start >= 1 && end <= 65535) {
                    for (int i = start; i <= end; i++) {
                        if (!ports.contains(i)) {
                            ports.append(i);
                        }
                    }
                }
            }
        } else {
            bool ok;
            int port = trimmed.toInt(&ok);
            if (ok && port >= 1 && port <= 65535 && !ports.contains(port)) {
                ports.append(port);
            }
        }
    }

    std::sort(ports.begin(), ports.end());
    return ports;
}

bool MainWindow::isValidTarget(const QString &target)
{
    QHostAddress address(target);
    if (!address.isNull()) {
        return true;
    }

    QRegularExpression hostnameRegex("^[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?(\\.[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?)*$");
    return hostnameRegex.match(target).hasMatch();
}

void MainWindow::applyTargetPreset(const QString &preset)
{
    if (preset == "localhost") {
        ui->lineEdit_target->setText("127.0.0.1");
    } else if (preset == "192.168.1.1") {
        ui->lineEdit_target->setText("192.168.1.1");
    } else if (preset == "8.8.8.8") {
        ui->lineEdit_target->setText("8.8.8.8");
    }
}

void MainWindow::applyPortPreset(const QString &preset)
{
    if (preset.contains("Web")) {
        ui->lineEdit_customPorts->setText("80,443,8080,8443");
    } else if (preset.contains("Mail")) {
        ui->lineEdit_customPorts->setText("25,110,143,993,995");
    } else if (preset.contains("Common")) {
        ui->lineEdit_customPorts->setText("21,22,23,25,53,80,110,443");
    } else if (preset.contains("All")) {
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(65535);
        ui->lineEdit_customPorts->clear();
    }
}

void MainWindow::on_comboBox_presets_currentTextChanged(const QString &text)
{
    if (text != "Quick Targets") {
        applyTargetPreset(text);
    }
}

void MainWindow::on_comboBox_portPresets_currentTextChanged(const QString &text)
{
    if (text != "Port Presets") {
        applyPortPreset(text);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    showAbout();
}

void MainWindow::on_actionGithub_triggered()
{
    openGithub();
}

void MainWindow::onPortResult(int port, const QString &status, const QString &service, const QString &banner, int responseTime)
{
    int row = ui->tableWidget_results->rowCount();
    ui->tableWidget_results->insertRow(row);

    QString protocol = (currentScanType == ScanType::UDP_SCAN) ? "UDP" : "TCP";

    ui->tableWidget_results->setItem(row, 0, new QTableWidgetItem(QString::number(port)));
    ui->tableWidget_results->setItem(row, 1, new QTableWidgetItem(protocol));
    ui->tableWidget_results->setItem(row, 2, new QTableWidgetItem(status));
    ui->tableWidget_results->setItem(row, 3, new QTableWidgetItem(service));
    ui->tableWidget_results->setItem(row, 4, new QTableWidgetItem(banner));
    ui->tableWidget_results->setItem(row, 5, new QTableWidgetItem(QString::number(responseTime) + " ms"));

    QTableWidgetItem *statusItem = ui->tableWidget_results->item(row, 2);
    if (status == "Open") {
        statusItem->setBackground(QBrush(QColor(144, 238, 144)));
        openPorts++;
    } else if (status == "Closed") {
        statusItem->setBackground(QBrush(QColor(255, 182, 193)));
    } else {
        statusItem->setBackground(QBrush(QColor(255, 255, 224)));
    }

    ui->tableWidget_results->resizeColumnsToContents();
}

void MainWindow::openGithub()
{
    QDesktopServices::openUrl(QUrl("https://github.com/CyberNilsen/CyberScanner"));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About CyberScanner",
                       "CyberScanner v1.0\n\n"
                       "Enhanced Port Scanner with multiple scan types,\n"
                       "OS detection, and service detection capabilities.\n\n"
                       "Features:\n"
                       "• Multiple scan types (TCP, UDP, SYN, etc.)\n"
                       "• Timing templates for stealth scanning\n"
                       "• OS detection capabilities\n"
                       "• Service detection and banner grabbing\n"
                       "• Export results to various formats\n"
                       "• Real-time scanning progress\n\n"
                       "Built with Qt and C++\n"
                       "Copyright © 2024");
}

QString PortScanner::buildNmapCommand(const QString &target, const QList<int> &ports)
{
    QString command = "nmap";

    switch (scanType) {
    case ScanType::TCP_SYN:
        command += " -sS";
        break;
    case ScanType::UDP_SCAN:
        command += " -sU";
        break;
    case ScanType::TCP_FIN:
        command += " -sF";
        break;
    case ScanType::TCP_XMAS:
        command += " -sX";
        break;
    case ScanType::TCP_NULL:
        command += " -sN";
        break;
    case ScanType::TCP_ACK:
        command += " -sA";
        break;
    case ScanType::TCP_WINDOW:
        command += " -sW";
        break;
    default:
        command += " -sT";
        break;
    }

    switch (timingTemplate) {
    case TimingTemplate::T0_PARANOID:
        command += " -T0";
        break;
    case TimingTemplate::T1_SNEAKY:
        command += " -T1";
        break;
    case TimingTemplate::T2_POLITE:
        command += " -T2";
        break;
    case TimingTemplate::T3_NORMAL:
        command += " -T3";
        break;
    case TimingTemplate::T4_AGGRESSIVE:
        command += " -T4";
        break;
    case TimingTemplate::T5_INSANE:
        command += " -T5";
        break;
    }

    if (enableServiceDetection) {
        command += " -sV";
    }

    if (enableOSDetection) {
        command += " -O";
    }

    if (enableAggressiveScan) {
        command += " -A";
    }

    if (!ports.isEmpty()) {
        command += " -p ";
        QStringList portStrings;
        for (int port : ports) {
            portStrings << QString::number(port);
        }
        command += portStrings.join(",");
    }

    command += " " + target;

    return command;
}

#include "mainwindow.h"
#include <QBrush>
#include <QColor>



#include "mainwindow.moc"
