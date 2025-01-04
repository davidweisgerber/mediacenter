#include <QTimer>
#include "debugwindow.h"
#include "ui_debugwindow.h"

DebugWindow::DebugWindow(char *dmxBuffer, QWidget *parent) :
    QDialog(parent)
    ,m_dmxBuffer(dmxBuffer)
    ,ui(new Ui::DebugWindow)
{
    ui->setupUi(this);

    for (int i=0; i < 512; i++)
    {
        auto *item = new QTreeWidgetItem(QStringList() << QString::number(i) << "");
        ui->treeWidget->addTopLevelItem(item);
    }

    ui->treeWidget->resizeColumnToContents(0);

    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DebugWindow::refresh);
    timer->setInterval(50);
    timer->start();
}

DebugWindow::~DebugWindow()
{
    delete ui;
}

void DebugWindow::refresh()
{
    for (int i=0; i < 512; i++)
    {
        ui->treeWidget->topLevelItem(i)->setText(1, QString::number(static_cast<unsigned char>(m_dmxBuffer[i])));
    }
}
