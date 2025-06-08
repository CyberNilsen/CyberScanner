#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application info
    a.setApplicationName("CyberScanner");
    a.setApplicationVersion("1");
    a.setOrganizationName("CyberNilsen");

    // Create icon with multiple PNG sizes for cross-platform support
    QIcon appIcon;
    appIcon.addFile(":/icons/Images/CyberScanner16x16.png", QSize(16, 16));
    appIcon.addFile(":/icons/Images/CyberScanner32x32.png", QSize(32, 32));
    appIcon.addFile(":/icons/Images/CyberScanner48x48.png", QSize(48, 48));
    appIcon.addFile(":/icons/Images/CyberScanner64x64.png", QSize(64, 64));
    appIcon.addFile(":/icons/Images/CyberScanner128x128.png", QSize(128, 128));
    appIcon.addFile(":/icons/Images/CyberScanner256x256.png", QSize(256, 256));

    // Set as default application icon
    a.setWindowIcon(appIcon);

    MainWindow w;
    w.show();

    return a.exec();
}
