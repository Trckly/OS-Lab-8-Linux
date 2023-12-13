#include "prioritywidget.hpp"
#include "ui_prioritywidget.h"

PriorityWidget::PriorityWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PriorityWidget)
{
    ui->setupUi(this);
}

PriorityWidget::~PriorityWidget()
{
    delete ui;
}

int PriorityWidget::GetNewPriority(){
    return ui->spinBox->value();
}
