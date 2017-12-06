#include <QtDebug>
#include <QInputDialog>
#include "eurolitepmd8configuration.h"
#include "lightfader.h"

LightFader::LightFader(int startChannel, QString name, OperatingMode mode, char *dmxData,  QWidget *parent)
	: QWidget(parent)
    , m_values()
    , m_startChannel(startChannel)
    , m_operatingMode(mode)
    , m_dmxData(dmxData)
    , m_masterValue(0)
{
	ui.setupUi(this);
    ui.faderName->setText( name );

    connect(ui.slider, &QSlider::valueChanged, this, &LightFader::setSliderValue);
    connect(ui.configureButton, &QAbstractButton::clicked, this, &LightFader::configureClicked);

    switch (m_operatingMode)
    {
        case SINGLE_CHANNEL:
            m_values.push_back(0);
            break;
        case EUROLITE_PMD_8:
            m_values = QVector<int>(27, 0);
            break;
    }
}

LightFader::~LightFader()
{

}

const QVector<int> &LightFader::getValues()
{
    return m_values;
}

int LightFader::getStartChannel()
{
    return m_startChannel;
}

void LightFader::setValue(int value, int channel)
{
    if (channel == 0 && m_operatingMode == SINGLE_CHANNEL)
    {
        ui.slider->setValue(static_cast<int>(static_cast<double>(value) / 2.55));
        return;
    }
    else if (channel == 1 && m_operatingMode == EUROLITE_PMD_8)
    {
        ui.slider->setValue(static_cast<int>(static_cast<double>(value) / 2.55));
        return;
    }

    setValueInternal(value, channel);
}

void LightFader::setValueInternal(int value, int channel)
{
    if (m_values.size() < channel)
    {
        qCritical() << "Channel bigger than channel size";
        return;
    }

    m_values[channel] = value;

    calculateValues();
}

void LightFader::setMasterValue(int value)
{
    m_masterValue = value;

    calculateValues();
}

LightFader::OperatingMode LightFader::getMode() const
{
    return m_operatingMode;
}

void LightFader::setSliderValue(int newValue)
{
    ui.faderStrength->setText(QString::number(newValue) + "%");

    int value = static_cast<int>(round(static_cast<double>(newValue) * 2.55));

    switch (m_operatingMode)
    {
        case SINGLE_CHANNEL:
            setValueInternal(value, 0);
            break;
        case EUROLITE_PMD_8:
            setValueInternal(value, 1);
            break;
    }
}

void LightFader::configureClicked()
{
    switch (m_operatingMode)
    {
        case SINGLE_CHANNEL:
        {
            QInputDialog *dlg = new QInputDialog(this);
            dlg->setInputMode(QInputDialog::IntInput);
            dlg->setIntMaximum(255);
            dlg->setIntMinimum(0);
            dlg->setIntValue(m_values[0]);
            dlg->setOption(QInputDialog::NoButtons, true);
            dlg->setWindowTitle(ui.faderName->text());
            dlg->setLabelText(tr("DMX Value:"));
            connect(dlg, &QInputDialog::intValueChanged, this, &LightFader::setValueFromDialog);
            dlg->exec();
            break;
        }
        case EUROLITE_PMD_8:
            EuroLitePMD8Configuration *dlg = new EuroLitePMD8Configuration(this);
            dlg->exec();
            break;
    }
}

void LightFader::setValueFromDialog(int value)
{
    setValue(value, 0);
}

void LightFader::calculateValues()
{
    switch (m_operatingMode)
    {
        case SINGLE_CHANNEL:
            calculateValuesSingle();
            break;
        case EUROLITE_PMD_8:
            calculateValuesPMD8();
            break;
    }
}

void LightFader::calculateValuesSingle()
{
    double value = m_values[0];
    value = value * m_masterValue;
    value = value / 100;

    m_dmxData[m_startChannel] = static_cast<char>(static_cast<unsigned char>(round(value)));
}

void LightFader::calculateValuesPMD8()
{
    m_dmxData[m_startChannel] = static_cast<char>(static_cast<unsigned char>(m_values[0]));

    if (m_values[0] < 136)
    {
        double value = m_values[1];
        value = value * m_masterValue;
        value = value / 100;

        m_dmxData[m_startChannel + 1] = static_cast<char>(static_cast<unsigned char>(round(value)));
    }
    else
    {
        m_dmxData[m_startChannel + 1] = static_cast<char>(static_cast<unsigned char>(m_values[1]));
    }

    for (int i=2; i < m_values.size(); i++)
    {
        m_dmxData[m_startChannel + i] = static_cast<char>(static_cast<unsigned char>(m_values[i]));
    }
}

QString LightFader::getName()
{
	return ui.faderName->text();
}
