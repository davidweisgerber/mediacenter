#include <QColorDialog>
#include "lightfader.h"
#include "eurolitepmd8configuration.h"
#include "ui_eurolitepmd8configuration.h"

EuroLitePMD8Configuration::EuroLitePMD8Configuration(LightFader *parentFader, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EuroLitePMD8Configuration),
    m_parent(parentFader)
{
    ui->setupUi(this);

    connect(ui->modeBox, &QComboBox::currentTextChanged, this, &EuroLitePMD8Configuration::modeChanged);
    connect(ui->dimmerBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &EuroLitePMD8Configuration::dimmerChanged);
    connect(ui->flashBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &EuroLitePMD8Configuration::flashChanged);
    connect(ui->colorButton1, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);
    connect(ui->colorButton2, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);
    connect(ui->colorButton3, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);
    connect(ui->colorButton4, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);
    connect(ui->colorButton5, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);
    connect(ui->colorButton6, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);
    connect(ui->colorButton7, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);
    connect(ui->colorButton8, &QAbstractButton::clicked, this, &EuroLitePMD8Configuration::changeColorPressed);

    buildLabels();
}

EuroLitePMD8Configuration::~EuroLitePMD8Configuration()
{
    delete ui;
}

void EuroLitePMD8Configuration::changeColorPressed()
{
    QObject *object = sender();
    QColorDialog dlg(this);
    dlg.exec();

    QColor color = dlg.selectedColor();
    if (color.isValid() == false)
    {
        return;
    }

    int number = object->objectName().remove("colorButton").toInt();
    const int start = ((number-1) * 3) + 3;
    m_parent->setValue(color.red(), start);
    m_parent->setValue(color.green(), start + 1);
    m_parent->setValue(color.blue(), start + 2);

    buildLabels();
}

void EuroLitePMD8Configuration::modeChanged()
{

}

void EuroLitePMD8Configuration::dimmerChanged()
{
    m_parent->setValue(ui->dimmerBox->value(), 1);
}

void EuroLitePMD8Configuration::flashChanged()
{
    m_parent->setValue(ui->flashBox->value(), 2);
}

void EuroLitePMD8Configuration::buildLabels()
{
    auto &values = m_parent->getValues();

    for (int i=1; i <= 8; i++)
    {
        const int start = ((i-1) * 3) + 3;
        QLabel *label = qobject_cast<QLabel *>(findChild<QLabel *>(QString("colorLabel%1").arg(i)));
        label->setText(QString("R: %1\tG: %2\tB: %3").arg(values[start]).arg(values[start+1]).arg(values[start+2]));
    }
}
