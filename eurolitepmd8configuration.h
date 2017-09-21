#ifndef EUROLITEPMD8CONFIGURATION_H
#define EUROLITEPMD8CONFIGURATION_H

#include <QDialog>

namespace Ui {
class EuroLitePMD8Configuration;
}

class EuroLitePMD8Configuration : public QDialog
{
    Q_OBJECT

public:
    explicit EuroLitePMD8Configuration(QWidget *parent = 0);
    ~EuroLitePMD8Configuration();

private:
    Ui::EuroLitePMD8Configuration *ui;
};

#endif // EUROLITEPMD8CONFIGURATION_H
