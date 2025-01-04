#ifndef EUROLITEPMD8CONFIGURATION_H
#define EUROLITEPMD8CONFIGURATION_H

#include <QDialog>

class LightFader;
class QColor;

namespace Ui
{
    class EuroLitePMD8Configuration;
}

class EuroLitePMD8Configuration : public QDialog
{
    Q_OBJECT

public:
    explicit EuroLitePMD8Configuration(LightFader *parentFader, QWidget *parent = nullptr);
    ~EuroLitePMD8Configuration() override;

private slots:
    void changeColorPressed();
    void modeChanged();
    void dimmerChanged();
    void flashChanged();
    void colorChanged(const QColor &color);

private:
    void buildLabels();

    enum class Mode
    {
        DIMMER = 0,
        R = 16,
        G = 24,
        B = 32,
        RG = 40,
        GB = 48,
        RB = 56,
        RGB = 64,
        COLOR_1 = 72,
        COLOR_2 = 80,
        COLOR_3 = 88,
        COLOR_4 = 96,
        COLOR_5 = 104,
        COLOR_6 = 112,
        COLOR_7 = 120,
        COLOR_8 = 128,
        DREAM = 136,
        METEOR = 144,
        FADE = 152,
        CHANGE = 160,
        FLOW_1 = 168,
        FLOW_2 = 176,
        FLOW_3 = 184,
        FLOW_4 = 192,
        FLOW_5 = 200,
        FLOW_6 = 208,
        FLOW_7 = 216,
        FLOW_8 = 224,
        FLOW_9 = 232,
        SOUND = 240
    };

    Ui::EuroLitePMD8Configuration *ui;
    LightFader *m_parent;
};

#endif // EUROLITEPMD8CONFIGURATION_H
