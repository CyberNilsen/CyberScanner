#include "mainwindow.h" // imports/includes mainwindow.h so it includes the header file.

#include <QApplication> // imports the qt framework.

int main(int argc, char *argv[]) // argc tells me how many command line arguments were passed, and char pointers.
{
    QApplication a(argc, argv); // Creates the qt application
    MainWindow w; // creates the window instance
    w.show(); // shows the window instance
    return a.exec(); // starts the applications event loop. This is to keep the program running.
}
