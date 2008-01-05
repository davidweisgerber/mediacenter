#include <QtGui/QApplication>
#include <QPlastiqueStyle>
#include "mediacenter.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setStyle( new QPlastiqueStyle() );
	mediacenter w;
	a.setQuitOnLastWindowClosed( false );
	return a.exec();
}
