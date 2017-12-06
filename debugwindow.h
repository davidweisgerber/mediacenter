#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QDialog>

namespace Ui {
class DebugWindow;
}

class DebugWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DebugWindow(char *dmxBuffer, QWidget *parent = 0);
    ~DebugWindow();

private slots:
    void refresh();

private:
    char *m_dmxBuffer;
    Ui::DebugWindow *ui;
};

#endif // DEBUGWINDOW_H
