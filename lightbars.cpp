#include "lightbars.h"
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QtDebug>

LightBars::LightBars(char *dmxBuffer, QWidget *parent)
    : QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint )
    , m_dmxBuffer(dmxBuffer)
    , m_master(100)
{
	setWindowTitle( tr("Light Faders") );
	setGeometry( QRect( 100, 100, 1024, 1024 ) );
	QWidget *wid = new QWidget( this );
	layout = new QVBoxLayout( );

	wid->setLayout( layout );
	layout->setSpacing(0);
	layout->setMargin(0);
}

LightBars::~LightBars()
{

}

void LightBars::buildUp(const QJsonObject &source)
{
	QMap<int, int> status;
	bool wasVisible = false;
	QPoint oldpos;
    if( isVisible() )
    {
		wasVisible = true;
		oldpos = pos();
		hide();
	}
	
    if( layout->count() > 0 )
    {
		status = getStatus();
	}

    QJsonArray faderArray = source["faders"].toArray();

    int num_faders = faderArray.size();
	
	QLayoutItem *child;
    while ((child = layout->takeAt(0)) != 0)
    {
		delete child->widget();
		delete child;
	}

    for( int i=0; i < num_faders; i++ )
    {
        int mode = faderArray[i].toObject()["mode"].toInt(0);
        int strength = faderArray[i].toObject()["strength"].toInt(0);

        LightFader *newFader = new LightFader(
            faderArray[i].toObject()["channel"].toInt(),
            faderArray[i].toObject()["name"].toString(),
            static_cast<LightFader::OperatingMode>(mode),
            m_dmxBuffer);

        if (mode == 0)
        {
            newFader->setValue(strength, 0);
        }
        else if (mode == 1)
        {
            newFader->setValue(strength, 1);
        }

		layout->addWidget( newFader );
	}

    QSize size( 210, 29*layout->count() + 29 );
	layout->parentWidget()->resize( size );

	setMinimumSize( size );
	setMaximumSize( size );

	resize( size );

    if( status.count() > 0 )
    {
		setStatus( status );
	}

    if( wasVisible )
    {
		move( oldpos );
		show();
    }
}

void LightBars::masterChanged(int newMaster)
{
    m_master = newMaster;

    for( int i=0; i < layout->count(); i++ )
    {
        LightFader *cur = qobject_cast<LightFader*>(layout->itemAt(i)->widget());
        cur->setMasterValue(newMaster);
    }
}

QMap<int, int> LightBars::getStatus()
{
	QMap<int, int> retVal;

    for( int i=0; i < layout->count(); i++ )
    {
        LightFader *cur = qobject_cast<LightFader*>(layout->itemAt(i)->widget());
        int startChannel = cur->getStartChannel();
        QVector<int> values = cur->getValues();

        for (int i=0; i < values.size(); i++)
        {
            retVal.insert(startChannel + i, values[i]);
        }
	}

	return retVal;
}

void LightBars::setStatus( QMap<int, int> status )
{
    for( int i=0; i < layout->count(); i++ )
    {
        LightFader *cur = qobject_cast<LightFader*>(layout->itemAt(i)->widget());
        int numberOfChannels = cur->getValues().size();

        for (int i=0; i < numberOfChannels; i++)
        {
            cur->setValue(status.value(cur->getStartChannel() + i, 0), i);
        }
    }
}

bool LightBars::isFaderMaster(int channel)
{
    for( int i=0; i < layout->count(); i++ )
    {
        LightFader *cur = qobject_cast<LightFader*>(layout->itemAt(i)->widget());
        if (cur->getMode() == LightFader::SINGLE_CHANNEL && channel == cur->getStartChannel())
        {
            return true;
        }
        else if (cur->getMode() == LightFader::EUROLITE_PMD_8 && channel == cur->getStartChannel() + 1)
        {
            return true;
        }
    }
}

void LightBars::showToggle()
{
	setVisible( !isVisible() );
}
