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

    ui->dimmerBox->setValue(m_parent->getValues()[1]);
    ui->flashBox->setValue(m_parent->getValues()[2]);

    ui->modeBox->setItemData(0, static_cast<int>(Mode::DIMMER));
    ui->modeBox->setItemData(1, static_cast<int>(Mode::R));
    ui->modeBox->setItemData(2, static_cast<int>(Mode::G));
    ui->modeBox->setItemData(3, static_cast<int>(Mode::B));
    ui->modeBox->setItemData(4, static_cast<int>(Mode::RG));
    ui->modeBox->setItemData(5, static_cast<int>(Mode::GB));
    ui->modeBox->setItemData(6, static_cast<int>(Mode::RB));
    ui->modeBox->setItemData(7, static_cast<int>(Mode::RGB));
    ui->modeBox->setItemData(8, static_cast<int>(Mode::COLOR_1));
    ui->modeBox->setItemData(9, static_cast<int>(Mode::COLOR_2));
    ui->modeBox->setItemData(10, static_cast<int>(Mode::COLOR_3));
    ui->modeBox->setItemData(11, static_cast<int>(Mode::COLOR_4));
    ui->modeBox->setItemData(12, static_cast<int>(Mode::COLOR_5));
    ui->modeBox->setItemData(13, static_cast<int>(Mode::COLOR_6));
    ui->modeBox->setItemData(14, static_cast<int>(Mode::COLOR_7));
    ui->modeBox->setItemData(15, static_cast<int>(Mode::COLOR_8));
    ui->modeBox->setItemData(16, static_cast<int>(Mode::DREAM));
    ui->modeBox->setItemData(17, static_cast<int>(Mode::METEOR));
    ui->modeBox->setItemData(18, static_cast<int>(Mode::FADE));
    ui->modeBox->setItemData(19, static_cast<int>(Mode::CHANGE));
    ui->modeBox->setItemData(20, static_cast<int>(Mode::FLOW_1));
    ui->modeBox->setItemData(21, static_cast<int>(Mode::FLOW_2));
    ui->modeBox->setItemData(22, static_cast<int>(Mode::FLOW_3));
    ui->modeBox->setItemData(23, static_cast<int>(Mode::FLOW_4));
    ui->modeBox->setItemData(24, static_cast<int>(Mode::FLOW_5));
    ui->modeBox->setItemData(25, static_cast<int>(Mode::FLOW_6));
    ui->modeBox->setItemData(26, static_cast<int>(Mode::FLOW_7));
    ui->modeBox->setItemData(27, static_cast<int>(Mode::FLOW_8));
    ui->modeBox->setItemData(28, static_cast<int>(Mode::FLOW_9));
    ui->modeBox->setItemData(29, static_cast<int>(Mode::SOUND));

    for (int i=0; i < ui->modeBox->count(); i++)
    {
        if (ui->modeBox->itemData(i).toInt() == m_parent->getValues()[0])
        {
            ui->modeBox->setCurrentIndex(i);
            break;
        }
    }

    connect(ui->modeBox, &QComboBox::currentTextChanged, this, &EuroLitePMD8Configuration::modeChanged);
}

EuroLitePMD8Configuration::~EuroLitePMD8Configuration()
{
    delete ui;
}

void EuroLitePMD8Configuration::changeColorPressed()
{
    QObject *object = sender();
    int number = object->objectName().remove("colorButton").toInt();
    const int start = ((number-1) * 3) + 3;
    QColor preselectedColor = QColor(m_parent->getValues()[start], m_parent->getValues()[start + 1], m_parent->getValues()[start + 2]);

    QColorDialog dlg(this);
    dlg.setObjectName(QString::number(start));
    dlg.setCurrentColor(preselectedColor);
    connect(&dlg, &QColorDialog::currentColorChanged, this, &EuroLitePMD8Configuration::colorChanged);
    dlg.exec();
}

void EuroLitePMD8Configuration::modeChanged()
{
    int value = ui->modeBox->currentData().toInt();

    m_parent->setValue(value, 0);
}

void EuroLitePMD8Configuration::dimmerChanged()
{
    m_parent->setValue(ui->dimmerBox->value(), 1);
}

void EuroLitePMD8Configuration::flashChanged()
{
    m_parent->setValue(ui->flashBox->value(), 2);
}

void EuroLitePMD8Configuration::colorChanged(const QColor &color)
{
    int start = sender()->objectName().toInt();
    m_parent->setValue(color.red(), start);
    m_parent->setValue(color.green(), start + 1);
    m_parent->setValue(color.blue(), start + 2);

    buildLabels();
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
