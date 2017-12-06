#ifndef LIGHTFADER_H
#define LIGHTFADER_H

#include <QWidget>
#include "ui_lightfader.h"

class LightFader : public QWidget
{
	Q_OBJECT

public:
    enum OperatingMode
    {
        SINGLE_CHANNEL,
        EUROLITE_PMD_8
    };

    LightFader(int startChannel, QString name, OperatingMode mode, char *dmxData, QWidget *parent = 0);
	~LightFader();

    const QVector<int> &getValues();
    int getStartChannel();

    void setValue(int value, int channel);
    void setMasterValue(int value);
    OperatingMode getMode() const;

    QString getName();

private slots:
    void setSliderValue(int newValue);
    void configureClicked();
    void setValueFromDialog(int value);

private:
    void setValueInternal(int value, int channel);
    void calculateValues();
    void calculateValuesSingle();
    void calculateValuesPMD8();

    QVector<int> m_values;
    int m_startChannel;
    OperatingMode m_operatingMode;
    char *m_dmxData;
    int m_masterValue;
	Ui::LightFaderClass ui;
};

#endif // LIGHTFADER_H
