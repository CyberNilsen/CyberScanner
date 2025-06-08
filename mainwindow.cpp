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


void MainWindow::showAbout()
{
    QMessageBox::about(
        this,
        tr("About Port Scanner"),
        tr("<h3>Port Scanner v1.0</h3>"
           "<p>A powerful network port scanning tool built with Qt.</p>"
           "<p><b>Features:</b></p>"
           "<ul>"
           "<li>TCP Connect, SYN, and UDP scanning</li>"
           "<li>Multi-threaded scanning</li>"
           "<li>Service banner grabbing</li>"
           "<li>Results export (CSV, XML, JSON)</li>"
           "<li>Customizable port ranges</li>"
           "</ul>"
           "<p><b>Built with Qt Framework</b></p>"
           "<p>© 2024 - Educational purposes only</p>")
        );
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
