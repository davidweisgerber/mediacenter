#include <QApplication>
#include "mediacenter.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    a.setStyle("fusion");
	mediacenter w;
	a.setQuitOnLastWindowClosed( false );
	return a.exec();
}
