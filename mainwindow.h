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
    void on_actionGithub_triggered();

    // Combo box slots for presets
    void on_comboBox_presets_currentTextChanged(const QString &text);
    void on_comboBox_portPresets_currentTextChanged(const QString &text);

private:
    Ui::MainWindow *ui;

    // Helper functions
    void showAbout();
    void openGithub();
    void addSampleResults();

    // Preset handling functions
    void applyTargetPreset(const QString &preset);
    void applyPortPreset(const QString &preset);
};

#endif // MAINWINDOW_H
