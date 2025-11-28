#include <QApplication>
#include <QDebug>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    // Windows console UTF-8 (可选)
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    QApplication app(argc, argv);
    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    MainWindow w;
    w.setWindowTitle(u8"数据结构可视化 - 线性结构/栈（示例）");
    w.resize(1000, 600);
    w.show();

    qDebug() << "App started.";
    return app.exec();
}
