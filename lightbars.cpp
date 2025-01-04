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
	auto *wid = new QWidget(this);
	m_layout = new QVBoxLayout();

	wid->setLayout( m_layout );
	m_layout->setSpacing(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
}

LightBars::~LightBars()
= default;

void LightBars::buildUp(const QJsonObject &source)
{
	QMap<int, int> status;
	bool wasVisible = false;
	QPoint oldpos;

    if (isVisible())
    {
		wasVisible = true;
		oldpos = pos();
		hide();
	}
	
    if (m_layout->count() > 0)
    {
		status = getStatus();
	}

    QJsonArray faderArray = source["faders"].toArray();

    qsizetype num_faders = faderArray.size();
	
	QLayoutItem *child;
    while ((child = m_layout->takeAt(0)) != nullptr)
    {
		delete child->widget();
		delete child;
	}

    for(int i=0; i < num_faders; i++)
    {
        int mode = faderArray[i].toObject()["mode"].toInt(0);
        int strength = faderArray[i].toObject()["strength"].toInt(0);

        auto *newFader = new LightFader(
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

		m_layout->addWidget( newFader );
	}

    QSize size( 210, 29*m_layout->count() + 29 );
	m_layout->parentWidget()->resize( size );

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

    for( int i=0; i < m_layout->count(); i++ )
    {
        auto *cur = qobject_cast<LightFader*>(m_layout->itemAt(i)->widget());
        cur->setMasterValue(newMaster);
    }
}

QMap<int, int> LightBars::getStatus()
{
	QMap<int, int> retVal;

    for( int i=0; i < m_layout->count(); i++ )
    {
        auto *cur = qobject_cast<LightFader*>(m_layout->itemAt(i)->widget());
        int startChannel = cur->getStartChannel();
        QVector<int> values = cur->getValues();

        for (int j=0; i < values.size(); i++)
        {
            retVal.insert(startChannel + j, values[j]);
        }
	}

	return retVal;
}

void LightBars::setStatus(const QMap<int, int>& status)
{
    for( int i=0; i < m_layout->count(); i++ )
    {
        auto *cur = qobject_cast<LightFader*>(m_layout->itemAt(i)->widget());
        qsizetype numberOfChannels = cur->getValues().size();

        for (int j=0; i < numberOfChannels; i++)
        {
            cur->setValue(status.value(cur->getStartChannel() + j, 0), j);
        }
    }
}

bool LightBars::isFaderMaster(int channel)
{
    for( int i=0; i < m_layout->count(); i++ )
    {
        auto *cur = qobject_cast<LightFader*>(m_layout->itemAt(i)->widget());

		if (cur->getMode() == LightFader::SINGLE_CHANNEL && channel == cur->getStartChannel())
		{
		    return true;
		}

    	if (cur->getMode() == LightFader::EUROLITE_PMD_8 && channel == cur->getStartChannel() + 1)
		{
		    return true;
		}
    }

    return false;
}

void LightBars::showToggle()
{
	setVisible( !isVisible() );
}
