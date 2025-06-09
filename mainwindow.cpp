#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThreadPool>
#include <QRunnable>
#include <QMutexLocker>
#include <algorithm>

class PortScanTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    PortScanTask(const QString &host, int port, int timeout, PortScanner *scanner)
        : host(host), port(port), timeout(timeout), scanner(scanner)
    {
        setAutoDelete(true);
    }

    void run() override
    {
        QElapsedTimer timer;
        timer.start();

        QTcpSocket socket;
        socket.connectToHost(host, port);

        bool connected = socket.waitForConnected(timeout);
        int responseTime = timer.elapsed();

        QString status;
        QString banner;
        QString service = getServiceName(port);

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
            case QAbstractSocket::UnknownSocketError:
                if (responseTime < (timeout / 2)) {
                    status = "Closed";
                } else {
                    status = "Filtered";
                }
                break;
            default:
                status = "Closed";
                break;
            }
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
    PortScanner *scanner;

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
        }
        return services.value(port, "Unknown");
    }

    QString grabBanner(QTcpSocket *socket, int port)
    {
        if (!socket || !socket->isOpen()) {
            return "";
        }

        if (socket->waitForReadyRead(500)) {
            QByteArray data = socket->readAll();
            QString banner = QString::fromUtf8(data).trimmed();

            banner = banner.remove(QRegularExpression("[\\x00-\\x1F\\x7F]"));
            if (banner.length() > 80) {
                banner = banner.left(80) + "...";
            }
            return banner;
        }
        return "";
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scanner(nullptr)
    , totalPorts(0)
    , scannedPorts(0)
    , openPorts(0)
{
    ui->setupUi(this);

    scanner = new PortScanner(this);

    connect(scanner, &PortScanner::scanStarted, this, &MainWindow::onScanStarted);
    connect(scanner, &PortScanner::scanFinished, this, &MainWindow::onScanFinished);
    connect(scanner, &PortScanner::scanProgress, this, &MainWindow::onScanProgress);
    connect(scanner, &PortScanner::portResult, this, &MainWindow::onPortResult);
    connect(scanner, &PortScanner::scanError, this, &MainWindow::onScanError);
    connect(scanner, &PortScanner::logMessage, this, &MainWindow::onLogMessage);

    ui->tableWidget_results->setColumnCount(6);
    QStringList headers;
    headers << "Port" << "Protocol" << "Status" << "Service" << "Banner" << "Response Time";
    ui->tableWidget_results->setHorizontalHeaderLabels(headers);

    ui->tableWidget_results->resizeColumnsToContents();

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateUI);

    addLogMessage("Fast Port Scanner initialized - Ready to scan");
    addLogMessage("Using multithreaded scanning for maximum speed");
}

MainWindow::~MainWindow()
{
    if (scanner && scanner->isScanning()) {
        scanner->stopScan();
    }
    delete ui;
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
    addLogMessage(QString("Starting fast scan of %1 ports on %2").arg(totalPorts).arg(target));
    addLogMessage(QString("Using %1 concurrent threads").arg(QThreadPool::globalInstance()->maxThreadCount()));
    scanner->startScan(target, ports);
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
    ui->label_status->setText("Status: Fast Scanning...");
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
    addLogMessage(QString("Fast scan completed. Total time: %1").arg(timeStr));
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

void MainWindow::onPortResult(int port, const QString &status, const QString &service, const QString &banner, int responseTime)
{
    if (status == "Open") {
        openPorts++;

        addLogMessage(QString("OPEN: Port %1 (%2) - %3ms").arg(port).arg(service).arg(responseTime));
    }

    int row = ui->tableWidget_results->rowCount();
    ui->tableWidget_results->insertRow(row);

    ui->tableWidget_results->setItem(row, 0, new QTableWidgetItem(QString::number(port)));
    ui->tableWidget_results->setItem(row, 1, new QTableWidgetItem("TCP"));
    ui->tableWidget_results->setItem(row, 2, new QTableWidgetItem(status));
    ui->tableWidget_results->setItem(row, 3, new QTableWidgetItem(service));
    ui->tableWidget_results->setItem(row, 4, new QTableWidgetItem(banner));
    ui->tableWidget_results->setItem(row, 5, new QTableWidgetItem(responseTime > 0 ? QString("%1ms").arg(responseTime) : "timeout"));

    QTableWidgetItem *statusItem = ui->tableWidget_results->item(row, 2);
    if (status == "Open") {
        statusItem->setBackground(QColor(144, 238, 144)); // Light green
        statusItem->setForeground(QColor(0, 100, 0)); // Dark green text
    } else if (status == "Closed") {
        statusItem->setBackground(QColor(255, 182, 193)); // Light red
        statusItem->setForeground(QColor(139, 0, 0)); // Dark red text
    } else {
        statusItem->setBackground(QColor(255, 255, 224)); // Light yellow
        statusItem->setForeground(QColor(139, 139, 0)); // Dark yellow text
    }

    applyFilters();

    if (row % 10 == 0) {
        ui->tableWidget_results->resizeColumnsToContents();
    }
}

void MainWindow::onScanError(const QString &error)
{
    addLogMessage(QString("Error: %1").arg(error));
    QMessageBox::warning(this, "Scan Error", error);
}

void MainWindow::onLogMessage(const QString &message)
{
    addLogMessage(message);
}

void MainWindow::updateUI()
{
    qint64 elapsed = scanTimer.elapsed();
    QString timeStr = QString("%1:%2.%3").arg(elapsed / 60000, 2, 10, QChar('0'))
                          .arg((elapsed % 60000) / 1000, 2, 10, QChar('0'))
                          .arg((elapsed % 1000) / 10, 2, 10, QChar('0'));

    double rate = elapsed > 0 ? (scannedPorts * 1000.0 / elapsed) : 0;

    ui->label_stats->setText(QString("Scanned: %1/%2 | Open: %3 | Rate: %4/s | Time: %5")
                                 .arg(scannedPorts).arg(totalPorts).arg(openPorts)
                                 .arg(rate, 0, 'f', 1).arg(timeStr));
}

void MainWindow::clearResults()
{
    ui->tableWidget_results->setRowCount(0);
    ui->progressBar->setValue(0);
    scannedPorts = 0;
    openPorts = 0;
    ui->label_stats->setText("Scanned: 0 | Open: 0 | Rate: 0.0/s | Time: 00:00.00");
}

void MainWindow::applyFilters()
{
    QString filterText = ui->lineEdit_filter->text().toLower();
    QString filterType = ui->comboBox_filterType->currentText();

    for (int i = 0; i < ui->tableWidget_results->rowCount(); ++i) {
        bool showRow = true;

        if (!filterText.isEmpty()) {
            bool matchFound = false;
            for (int j = 0; j < ui->tableWidget_results->columnCount(); ++j) {
                QTableWidgetItem *item = ui->tableWidget_results->item(i, j);
                if (item && item->text().toLower().contains(filterText)) {
                    matchFound = true;
                    break;
                }
            }
            if (!matchFound) showRow = false;
        }

        if (showRow && filterType != "All") {
            QTableWidgetItem *statusItem = ui->tableWidget_results->item(i, 2);
            if (statusItem) {
                QString status = statusItem->text();
                if (filterType == "Open Only" && status != "Open") {
                    showRow = false;
                } else if (filterType == "Closed Only" && status != "Closed") {
                    showRow = false;
                }
            }
        }

        ui->tableWidget_results->setRowHidden(i, !showRow);
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
                if (ok1 && ok2 && start <= end && start > 0 && end <= 65535) {
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
            if (ok && port > 0 && port <= 65535) {
                if (!ports.contains(port)) {
                    ports.append(port);
                }
            }
        }
    }

    std::sort(ports.begin(), ports.end());
    return ports;
}

bool MainWindow::isValidTarget(const QString &target)
{
    QRegularExpression ipRegex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if (ipRegex.match(target).hasMatch()) {
        return true;
    }

    QRegularExpression hostnameRegex("^[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?(\\.([a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?))*$");
    return hostnameRegex.match(target).hasMatch();
}

void MainWindow::addLogMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString logEntry = QString("[%1] %2").arg(timestamp).arg(message);
    ui->textEdit_log->append(logEntry);

    if (ui->checkBox_autoScroll->isChecked()) {
        ui->textEdit_log->moveCursor(QTextCursor::End);
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

void MainWindow::on_comboBox_presets_currentTextChanged(const QString &text)
{
    applyTargetPreset(text);
}

void MainWindow::on_comboBox_portPresets_currentTextChanged(const QString &text)
{
    applyPortPreset(text);
}

void MainWindow::applyTargetPreset(const QString &preset)
{
    if (preset == "Quick Targets") {
        return;
    }

    if (preset == "localhost") {
        ui->lineEdit_target->setText("127.0.0.1");
    }
    else if (preset == "192.168.1.1") {
        ui->lineEdit_target->setText("192.168.1.1");
    }
    else if (preset == "8.8.8.8") {
        ui->lineEdit_target->setText("8.8.8.8");
    }
}

void MainWindow::applyPortPreset(const QString &preset)
{
    if (preset == "Port Presets") {
        return;
    }

    ui->lineEdit_customPorts->clear();

    if (preset == "Web (80,443,8080,8443)") {
        ui->lineEdit_customPorts->setText("80,443,8080,8443");
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(1);
    }
    else if (preset == "Mail (25,110,143,993,995)") {
        ui->lineEdit_customPorts->setText("25,110,143,993,995");
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(1);
    }
    else if (preset == "Common (21,22,23,25,53,80,110,443)") {
        ui->lineEdit_customPorts->setText("21,22,23,25,53,80,110,443,135,139,445,1433,3306,3389");
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(1);
    }
    else if (preset == "All (1-65535)") {
        ui->lineEdit_customPorts->clear();
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(65535);
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(
        this,
        tr("About Fast Port Scanner"),
        tr("<h3>Fast Port Scanner v2.0</h3>"
           "<p>A high-speed multithreaded network port scanning tool built with Qt.</p>"
           "<p><b>Features:</b></p>"
           "<ul>"
           "<li>Multithreaded TCP scanning for maximum speed</li>"
           "<li>Real-time results with progress tracking</li>"
           "<li>Service banner grabbing</li>"
           "<li>Customizable port ranges and timeout</li>"
           "<li>Advanced filtering and result export</li>"
           "</ul>"
           "<p><b>Performance:</b> Can scan thousands of ports per second</p>"
           "<p><b>Built with Qt Framework</b></p>"
           "<p>© 2025 - Educational purposes only</p>")
        );
}

void MainWindow::openGithub()
{
    QString githubUrl = "https://github.com/CyberNilsen";

    if (!QDesktopServices::openUrl(QUrl(githubUrl))) {
        QMessageBox::warning(
            this,
            tr("Error"),
            tr("Could not open GitHub link. Please visit: %1").arg(githubUrl)
            );
    }
}

void MainWindow::addSampleResults()
{

}

PortScanner::PortScanner(QObject *parent)
    : QObject(parent)
    , scanning(false)
    , connectionTimeout(1000)
    , completedScans(0)
{
    int optimalThreads = QThread::idealThreadCount() * 4;
    QThreadPool::globalInstance()->setMaxThreadCount(optimalThreads);
}

PortScanner::~PortScanner()
{
    stopScan();
}

void PortScanner::startScan(const QString &target, const QList<int> &ports)
{
    if (scanning) {
        return;
    }

    targetHost = target;
    portList = ports;
    completedScans = 0;
    scanning = true;

    emit scanStarted();
    emit logMessage(QString("Starting multithreaded scan with %1 threads").arg(QThreadPool::globalInstance()->maxThreadCount()));

    for (int port : ports) {
        PortScanTask *task = new PortScanTask(target, port, connectionTimeout, this);
        QThreadPool::globalInstance()->start(task);
    }
}

void PortScanner::stopScan()
{
    if (!scanning) {
        return;
    }

    scanning = false;

    QThreadPool::globalInstance()->clear();
    QThreadPool::globalInstance()->waitForDone(5000);

    emit scanFinished();
}

void PortScanner::portScanned(int port, const QString &status, const QString &service, const QString &banner, int responseTime)
{
    if (!scanning) {
        return;
    }

    completedScans++;

    emit portResult(port, status, service, banner, responseTime);
    emit scanProgress(completedScans, portList.size());

    if (completedScans >= portList.size()) {
        scanning = false;
        emit scanFinished();
    }
}

bool PortScanner::isScanning() const
{
    return scanning;
}

#include "mainwindow.moc"
