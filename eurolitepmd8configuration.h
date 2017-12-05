#ifndef EUROLITEPMD8CONFIGURATION_H
#define EUROLITEPMD8CONFIGURATION_H

#include <QDialog>

class LightFader;

namespace Ui {
class EuroLitePMD8Configuration;
}

class EuroLitePMD8Configuration : public QDialog
{
    Q_OBJECT

public:
    explicit EuroLitePMD8Configuration(LightFader *parentFader, QWidget *parent = 0);
    ~EuroLitePMD8Configuration();

private slots:
    void changeColorPressed();
    void modeChanged();
    void dimmerChanged();
    void flashChanged();

private:
    void buildLabels();

    Ui::EuroLitePMD8Configuration *ui;
    LightFader *m_parent;
};

#endif // EUROLITEPMD8CONFIGURATION_H
