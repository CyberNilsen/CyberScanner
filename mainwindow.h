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

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File menu slots
    void on_actionSave_Results_triggered();
    void on_actionLoad_Results_triggered();
    void on_actionExit_triggered();

    // Tools menu slots
    void on_actionExport_CSV_triggered();
    void on_actionExport_XML_triggered();
    void on_actionSettings_triggered();

    // Help menu slots
    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;

    // Helper functions
    void saveResultsToJson(const QString &filename);
    void loadResultsFromJson(const QString &filename);
    void exportToCsv(const QString &filename);
    void exportToXml(const QString &filename);
    void showSettings();
    void showAbout();

    // Add sample data for demonstration
    void addSampleResults();
};

#endif // MAINWINDOW_H
