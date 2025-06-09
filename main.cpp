#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("CyberScanner");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("CyberNilsen");
    a.setApplicationDisplayName("CyberScanner");
    QIcon appIcon;

    appIcon.addFile(":/icons/Images/CyberScanner16x16.png", QSize(16, 16));
    appIcon.addFile(":/icons/Images/CyberScanner24x24.png", QSize(24, 24));
    appIcon.addFile(":/icons/Images/CyberScanner32x32.png", QSize(32, 32));
    appIcon.addFile(":/icons/Images/CyberScanner48x48.png", QSize(48, 48));
    appIcon.addFile(":/icons/Images/CyberScanner64x64.png", QSize(64, 64));
    appIcon.addFile(":/icons/Images/CyberScanner128x128.png", QSize(128, 128));
    appIcon.addFile(":/icons/Images/CyberScanner256x256.png", QSize(256, 256));
    appIcon.addFile(":/icons/Images/CyberScanner512x512.png", QSize(512, 512));

    a.setWindowIcon(appIcon);


    MainWindow w;

    w.setWindowIcon(appIcon);

    w.show();

    return a.exec();
}
