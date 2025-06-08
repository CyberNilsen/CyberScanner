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
#include <QDesktopServices>  // Add this for opening URLs
#include <QUrl>              // Add this for URL handling

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
    // Help menu slots
    void on_actionAbout_triggered();
    void on_actionGithub_triggered();  // Add this slot for GitHub action

private:
    Ui::MainWindow *ui;

    // Helper functions
    void showAbout();
    void openGithub();  // Add this helper function

    // Add sample data for demonstration
    void addSampleResults();
};

#endif // MAINWINDOW_H
