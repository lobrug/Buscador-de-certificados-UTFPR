#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Injeta CSS global na aplicação
    a.setStyleSheet(
        "QMainWindow { background-color: #1e1e1e; }"
        "QLabel { color: #ffffff; font-size: 24px; font-weight: bold; }"
        "QPushButton { background-color: #0d47a1; color: white; border-radius: 4px; padding: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1565c0; }"
        "QPushButton:disabled { background-color: #424242; color: #757575; }"
        "QLineEdit, QComboBox { background-color: #2d2d2d; color: white; border: 1px solid #444; padding: 5px; border-radius: 3px; }"
        "QPlainTextEdit { background-color: #0c0c0c; color: #00ff00; font-family: 'Consolas'; font-size: 13px; border: 1px solid #333; }"
        );

    MainWindow w;
    w.show();
    return a.exec();
}