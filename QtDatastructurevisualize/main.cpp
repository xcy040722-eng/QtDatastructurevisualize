#include "QtDatastructurevisualize.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    // 在 Windows 控制台下强制使用 UTF-8 编码，避免 qDebug 中文乱码
    // （仅影响控制台编码，Qt 输出窗口一般可直接显示 Unicode）
    system("chcp 65001 > nul");

    QApplication app(argc, argv);

    // 设置全局字体（确保界面中文不出现方块）
    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    MainWindow w;
    w.setWindowTitle(QStringLiteral("数据结构可视化模拟器 - Phase1"));
    w.show();

    qDebug().noquote() << u8"程序启动：中文显示测试通过。";

    return app.exec();
}
