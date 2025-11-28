#include <QApplication>
#include <QFont>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    // Windows 控制台输出乱码修复
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    QApplication app(argc, argv);

    // 设置全局字体，防止中文乱码
    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    MainWindow w;
    w.setWindowTitle(QStringLiteral("数据结构可视化模拟器 - Phase 2 (完美线性版)"));
    w.resize(1024, 768);
    w.show();

    return app.exec();
}
