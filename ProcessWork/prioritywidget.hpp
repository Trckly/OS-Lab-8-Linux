#pragma once

#include <QDialog>

namespace Ui {
class PriorityWidget;
}

class PriorityWidget : public QDialog
{
    Q_OBJECT

public:
    explicit PriorityWidget(QWidget *parent = nullptr);
    ~PriorityWidget();

    int GetNewPriority();

private:
    Ui::PriorityWidget *ui;
};

