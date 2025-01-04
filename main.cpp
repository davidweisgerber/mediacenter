#include <QApplication>
#include "Mediacenter.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    a.setStyle("fusion");
	Mediacenter w;
	a.setQuitOnLastWindowClosed( false );
	return a.exec();
}
