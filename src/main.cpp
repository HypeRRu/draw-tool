#include <iostream>
#include <QApplication>
#include "./gui/interface.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);


	Interface window;

	window.resize(800, 600);
	window.setWindowTitle("PNG drawer");
	window.show();

	QObject::connect(&app, SIGNAL(aboutToQuit()), &window, SLOT(exitHandler()));
	window.setWindowIcon(QIcon("./icons/icon.png")); 

	return app.exec();
}