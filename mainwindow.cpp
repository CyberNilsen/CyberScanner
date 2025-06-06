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

// File Menu Functions
void MainWindow::on_actionSave_Results_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Scan Results"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/scan_results.json",
        tr("JSON Files (*.json);;All Files (*)")
        );

    if (!fileName.isEmpty()) {
        saveResultsToJson(fileName);
        ui->textEdit_log->append(QString("[INFO] Results saved to: %1").arg(fileName));
    }
}

void MainWindow::on_actionLoad_Results_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Load Scan Results"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("JSON Files (*.json);;All Files (*)")
        );

    if (!fileName.isEmpty()) {
        loadResultsFromJson(fileName);
        ui->textEdit_log->append(QString("[INFO] Results loaded from: %1").arg(fileName));
    }
}

void MainWindow::on_actionExit_triggered()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Exit Application"),
        tr("Are you sure you want to exit?"),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

// Tools Menu Functions
void MainWindow::on_actionExport_CSV_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export to CSV"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/scan_results.csv",
        tr("CSV Files (*.csv);;All Files (*)")
        );

    if (!fileName.isEmpty()) {
        exportToCsv(fileName);
        ui->textEdit_log->append(QString("[INFO] Results exported to CSV: %1").arg(fileName));
    }
}

void MainWindow::on_actionExport_XML_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export to XML"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/scan_results.xml",
        tr("XML Files (*.xml);;All Files (*)")
        );

    if (!fileName.isEmpty()) {
        exportToXml(fileName);
        ui->textEdit_log->append(QString("[INFO] Results exported to XML: %1").arg(fileName));
    }
}

void MainWindow::on_actionSettings_triggered()
{
    showSettings();
}

// Help Menu Functions
void MainWindow::on_actionAbout_triggered()
{
    showAbout();
}

// Helper Functions Implementation
void MainWindow::saveResultsToJson(const QString &filename)
{
    QJsonObject rootObject;
    QJsonArray resultsArray;

    // Save scan configuration
    QJsonObject configObject;
    configObject["target"] = ui->lineEdit_target->text();
    configObject["portFrom"] = ui->spinBox_portFrom->value();
    configObject["portTo"] = ui->spinBox_portTo->value();
    configObject["customPorts"] = ui->lineEdit_customPorts->text();
    configObject["scanType"] = ui->comboBox_scanType->currentText();
    configObject["timeout"] = ui->spinBox_timeout->value();
    configObject["threads"] = ui->spinBox_threads->value();
    configObject["grabBanners"] = ui->checkBox_serviceBanner->isChecked();
    rootObject["configuration"] = configObject;

    // Save results table data
    for (int row = 0; row < ui->tableWidget_results->rowCount(); ++row) {
        QJsonObject resultObject;
        for (int col = 0; col < ui->tableWidget_results->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidget_results->item(row, col);
            if (item) {
                QString columnName = ui->tableWidget_results->horizontalHeaderItem(col)->text();
                resultObject[columnName.toLower().replace(" ", "_")] = item->text();
            }
        }
        resultsArray.append(resultObject);
    }
    rootObject["results"] = resultsArray;

    // Save to file
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(rootObject);
        file.write(doc.toJson());
        file.close();
        QMessageBox::information(this, tr("Save Results"), tr("Results saved successfully!"));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not save file: %1").arg(file.errorString()));
    }
}

void MainWindow::loadResultsFromJson(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file: %1").arg(file.errorString()));
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject rootObject = doc.object();

    // Load configuration
    if (rootObject.contains("configuration")) {
        QJsonObject config = rootObject["configuration"].toObject();
        ui->lineEdit_target->setText(config["target"].toString());
        ui->spinBox_portFrom->setValue(config["portFrom"].toInt());
        ui->spinBox_portTo->setValue(config["portTo"].toInt());
        ui->lineEdit_customPorts->setText(config["customPorts"].toString());
        ui->spinBox_timeout->setValue(config["timeout"].toInt());
        ui->spinBox_threads->setValue(config["threads"].toInt());
        ui->checkBox_serviceBanner->setChecked(config["grabBanners"].toBool());

        // Set scan type combobox
        QString scanType = config["scanType"].toString();
        int index = ui->comboBox_scanType->findText(scanType);
        if (index >= 0) ui->comboBox_scanType->setCurrentIndex(index);
    }

    // Load results
    if (rootObject.contains("results")) {
        QJsonArray results = rootObject["results"].toArray();
        ui->tableWidget_results->setRowCount(results.size());

        for (int i = 0; i < results.size(); ++i) {
            QJsonObject result = results[i].toObject();
            ui->tableWidget_results->setItem(i, 0, new QTableWidgetItem(result["port"].toString()));
            ui->tableWidget_results->setItem(i, 1, new QTableWidgetItem(result["protocol"].toString()));
            ui->tableWidget_results->setItem(i, 2, new QTableWidgetItem(result["status"].toString()));
            ui->tableWidget_results->setItem(i, 3, new QTableWidgetItem(result["service"].toString()));
            ui->tableWidget_results->setItem(i, 4, new QTableWidgetItem(result["banner"].toString()));
            ui->tableWidget_results->setItem(i, 5, new QTableWidgetItem(result["response_time"].toString()));
        }
    }

    file.close();
    QMessageBox::information(this, tr("Load Results"), tr("Results loaded successfully!"));
}

void MainWindow::exportToCsv(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not create CSV file: %1").arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);

    // Write headers
    QStringList headers;
    for (int col = 0; col < ui->tableWidget_results->columnCount(); ++col) {
        headers << ui->tableWidget_results->horizontalHeaderItem(col)->text();
    }
    stream << headers.join(",") << "\n";

    // Write data
    for (int row = 0; row < ui->tableWidget_results->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < ui->tableWidget_results->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidget_results->item(row, col);
            rowData << (item ? item->text() : "");
        }
        stream << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Export CSV"), tr("Data exported to CSV successfully!"));
}

void MainWindow::exportToXml(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not create XML file: %1").arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    stream << "<scan_results>\n";
    stream << "  <configuration>\n";
    stream << "    <target>" << ui->lineEdit_target->text() << "</target>\n";
    stream << "    <port_range>" << ui->spinBox_portFrom->value() << "-" << ui->spinBox_portTo->value() << "</port_range>\n";
    stream << "    <scan_type>" << ui->comboBox_scanType->currentText() << "</scan_type>\n";
    stream << "  </configuration>\n";
    stream << "  <results>\n";

    for (int row = 0; row < ui->tableWidget_results->rowCount(); ++row) {
        stream << "    <port>\n";
        for (int col = 0; col < ui->tableWidget_results->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidget_results->item(row, col);
            QString columnName = ui->tableWidget_results->horizontalHeaderItem(col)->text().toLower().replace(" ", "_");
            stream << "      <" << columnName << ">" << (item ? item->text() : "") << "</" << columnName << ">\n";
        }
        stream << "    </port>\n";
    }

    stream << "  </results>\n";
    stream << "</scan_results>\n";

    file.close();
    QMessageBox::information(this, tr("Export XML"), tr("Data exported to XML successfully!"));
}

void MainWindow::showSettings()
{
    QMessageBox::information(
        this,
        tr("Settings"),
        tr("Settings dialog would be implemented here.\n\n"
           "This could include:\n"
           "• Default scan parameters\n"
           "• UI theme preferences\n"
           "• Network interface selection\n"
           "• Advanced scanning options\n"
           "• Export format preferences")
        );
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
