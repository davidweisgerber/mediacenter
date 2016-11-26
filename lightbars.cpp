#include "lightbars.h"
#include <QVBoxLayout>
#include <QSettings>
#include <QJsonArray>
#include <QJsonObject>

LightBars::LightBars(QWidget *parent)
    : QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint )
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

void LightBars::buildUp(const QJsonObject &source) {
	QMap<int, int> status;
	bool wasVisible = false;
	QPoint oldpos;
	if( isVisible() ) {
		wasVisible = true;
		oldpos = pos();
		hide();
	}
	
	if( layout->count() > 0 ) {
		status = getStatus();
	}

    QJsonArray faderArray = source["faders"].toArray();

    int num_faders = faderArray.size();
	
	QLayoutItem *child;
	while ((child = layout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}

	for( int i=0; i < num_faders; i++ ) {
        LightFader *newFader = new LightFader(
            faderArray[i].toObject()["channel"].toInt(),
            faderArray[i].toObject()["strength"].toInt(),
            faderArray[i].toObject()["name"].toString());
		layout->addWidget( newFader );
	}

    QSize size( 210, 29*layout->count() + 29 );
	layout->parentWidget()->resize( size );

	setMinimumSize( size );
	setMaximumSize( size );

	resize( size );

	if( status.count() > 0 ) {
		setStatus( status );
	}

	if( wasVisible ) {
		move( oldpos );
		show();
	}
}

QMap<int, int> LightBars::getStatus() {
	QMap<int, int> retVal;

	for( int i=0; i < layout->count(); i++ ) {
		LightFader *cur = (LightFader*)layout->itemAt(i)->widget();
		retVal.insert( cur->getChannel(), cur->getValue() );
	}

	return retVal;
}

void LightBars::setStatus( QMap<int, int> status ) {
	for( int i=0; i < layout->count(); i++ ) {
		LightFader *cur = (LightFader*)layout->itemAt(i)->widget();
		cur->setValue( status.value( cur->getChannel(), 0 ) );
	}
}

void LightBars::showToggle() {
	setVisible( !isVisible() );
}
