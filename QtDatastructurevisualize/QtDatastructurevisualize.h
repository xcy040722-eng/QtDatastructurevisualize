#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtDatastructurevisualize.h"

class QtDatastructurevisualize : public QMainWindow
{
    Q_OBJECT

public:
    QtDatastructurevisualize(QWidget *parent = nullptr);
    ~QtDatastructurevisualize();

private:
    Ui::QtDatastructurevisualizeClass ui;
};

