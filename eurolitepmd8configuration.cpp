#include "eurolitepmd8configuration.h"
#include "ui_eurolitepmd8configuration.h"

EuroLitePMD8Configuration::EuroLitePMD8Configuration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EuroLitePMD8Configuration)
{
    ui->setupUi(this);
}

EuroLitePMD8Configuration::~EuroLitePMD8Configuration()
{
    delete ui;
}
