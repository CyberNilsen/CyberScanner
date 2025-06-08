#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application info (important for Windows and macOS)
    a.setApplicationName("CyberScanner");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("CyberNilsen");
    a.setApplicationDisplayName("CyberScanner"); // For better display name

    // Create comprehensive icon with multiple sizes for all platforms
    QIcon appIcon;

    // Standard sizes for Windows taskbar, macOS dock, and Linux
    appIcon.addFile(":/icons/Images/CyberScanner16x16.png", QSize(16, 16));
    appIcon.addFile(":/icons/Images/CyberScanner24x24.png", QSize(24, 24)); // Linux panel
    appIcon.addFile(":/icons/Images/CyberScanner32x32.png", QSize(32, 32));
    appIcon.addFile(":/icons/Images/CyberScanner48x48.png", QSize(48, 48));
    appIcon.addFile(":/icons/Images/CyberScanner64x64.png", QSize(64, 64));
    appIcon.addFile(":/icons/Images/CyberScanner128x128.png", QSize(128, 128));
    appIcon.addFile(":/icons/Images/CyberScanner256x256.png", QSize(256, 256));
    appIcon.addFile(":/icons/Images/CyberScanner512x512.png", QSize(512, 512)); // macOS Retina

    // Set as default application icon (affects taskbar, dock, and window decorations)
    a.setWindowIcon(appIcon);

    // Also set a fallback icon path for the application
    // This helps with system integration on some Linux distributions

    MainWindow w;

    // Explicitly set the window icon as well (redundant but ensures compatibility)
    w.setWindowIcon(appIcon);

    w.show();

    return a.exec();
}
