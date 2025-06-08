#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set up the results table headers
    ui->tableWidget_results->setColumnCount(6);
    QStringList headers;
    headers << "Port" << "Protocol" << "Status" << "Service" << "Banner" << "Response Time";
    ui->tableWidget_results->setHorizontalHeaderLabels(headers);

    // Resize columns to content
    ui->tableWidget_results->resizeColumnsToContents();

    // Add some sample data for demonstration
    addSampleResults();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Help Menu Functions
void MainWindow::on_actionAbout_triggered()
{
    showAbout();
}

// GitHub Menu Function
void MainWindow::on_actionGithub_triggered()
{
    openGithub();
}

// Target preset combo box handler
void MainWindow::on_comboBox_presets_currentTextChanged(const QString &text)
{
    applyTargetPreset(text);
}

// Port preset combo box handler
void MainWindow::on_comboBox_portPresets_currentTextChanged(const QString &text)
{
    applyPortPreset(text);
}

void MainWindow::applyTargetPreset(const QString &preset)
{
    // Don't change anything if "Quick Targets" is selected (it's the default/placeholder)
    if (preset == "Quick Targets") {
        return;
    }

    // Apply the selected target to the target input field
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
    // Don't change anything if "Port Presets" is selected (it's the default/placeholder)
    if (preset == "Port Presets") {
        return;
    }

    // Clear the custom ports field and set port range or custom ports based on selection
    ui->lineEdit_customPorts->clear();

    if (preset == "Web (80,443,8080,8443)") {
        ui->lineEdit_customPorts->setText("80,443,8080,8443");
        // Clear the range fields when using custom ports
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(1);
    }
    else if (preset == "Mail (25,110,143,993,995)") {
        ui->lineEdit_customPorts->setText("25,110,143,993,995");
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(1);
    }
    else if (preset == "Common (21,22,23,25,53,80,110,443)") {
        ui->lineEdit_customPorts->setText("21,22,23,25,53,80,110,443");
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(1);
    }
    else if (preset == "All (1-65535)") {
        // For "All ports", use the range instead of custom ports
        ui->lineEdit_customPorts->clear();
        ui->spinBox_portFrom->setValue(1);
        ui->spinBox_portTo->setValue(65535);
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(
        this,
        tr("About Port Scanner"),
        tr("<h3>Port Scanner v1.0</h3>"
           "<p>A network port scanning tool built with Qt.</p>"
           "<p><b>Features:</b></p>"
           "<ul>"
           "<li>TCP scanning</li>"
           "<li>Multi-threaded scanning</li>"
           "<li>Service banner grabbing</li>"
           "<li>Customizable port ranges</li>"
           "</ul>"
           "<p><b>Built with Qt Framework</b></p>"
           "<p>© 2025 - Educational purposes only</p>")
        );
}

void MainWindow::openGithub()
{
    // Replace "yourusername" with your actual GitHub username
    QString githubUrl = "https://github.com/CyberNilsen";

    // Open the URL in the default web browser
    if (!QDesktopServices::openUrl(QUrl(githubUrl))) {
        // If opening fails, show an error message
        QMessageBox::warning(
            this,
            tr("Error"),
            tr("Could not open GitHub link. Please visit: %1").arg(githubUrl)
            );
    }
}

void MainWindow::addSampleResults()
{
    // Add some sample data to demonstrate the functionality
    ui->tableWidget_results->setRowCount(5);

    // Sample row 1
    ui->tableWidget_results->setItem(0, 0, new QTableWidgetItem("22"));
    ui->tableWidget_results->setItem(0, 1, new QTableWidgetItem("TCP"));
    ui->tableWidget_results->setItem(0, 2, new QTableWidgetItem("Open"));
    ui->tableWidget_results->setItem(0, 3, new QTableWidgetItem("SSH"));
    ui->tableWidget_results->setItem(0, 4, new QTableWidgetItem("OpenSSH 8.0"));
    ui->tableWidget_results->setItem(0, 5, new QTableWidgetItem("15ms"));

    // Sample row 2
    ui->tableWidget_results->setItem(1, 0, new QTableWidgetItem("80"));
    ui->tableWidget_results->setItem(1, 1, new QTableWidgetItem("TCP"));
    ui->tableWidget_results->setItem(1, 2, new QTableWidgetItem("Open"));
    ui->tableWidget_results->setItem(1, 3, new QTableWidgetItem("HTTP"));
    ui->tableWidget_results->setItem(1, 4, new QTableWidgetItem("Apache/2.4.41"));
    ui->tableWidget_results->setItem(1, 5, new QTableWidgetItem("23ms"));

    // Sample row 3
    ui->tableWidget_results->setItem(2, 0, new QTableWidgetItem("443"));
    ui->tableWidget_results->setItem(2, 1, new QTableWidgetItem("TCP"));
    ui->tableWidget_results->setItem(2, 2, new QTableWidgetItem("Open"));
    ui->tableWidget_results->setItem(2, 3, new QTableWidgetItem("HTTPS"));
    ui->tableWidget_results->setItem(2, 4, new QTableWidgetItem("nginx/1.18.0"));
    ui->tableWidget_results->setItem(2, 5, new QTableWidgetItem("18ms"));

    // Sample row 4
    ui->tableWidget_results->setItem(3, 0, new QTableWidgetItem("3389"));
    ui->tableWidget_results->setItem(3, 1, new QTableWidgetItem("TCP"));
    ui->tableWidget_results->setItem(3, 2, new QTableWidgetItem("Closed"));
    ui->tableWidget_results->setItem(3, 3, new QTableWidgetItem("RDP"));
    ui->tableWidget_results->setItem(3, 4, new QTableWidgetItem(""));
    ui->tableWidget_results->setItem(3, 5, new QTableWidgetItem("1000ms"));

    // Sample row 5
    ui->tableWidget_results->setItem(4, 0, new QTableWidgetItem("21"));
    ui->tableWidget_results->setItem(4, 1, new QTableWidgetItem("TCP"));
    ui->tableWidget_results->setItem(4, 2, new QTableWidgetItem("Filtered"));
    ui->tableWidget_results->setItem(4, 3, new QTableWidgetItem("FTP"));
    ui->tableWidget_results->setItem(4, 4, new QTableWidgetItem(""));
    ui->tableWidget_results->setItem(4, 5, new QTableWidgetItem("timeout"));

    ui->tableWidget_results->resizeColumnsToContents();
}
