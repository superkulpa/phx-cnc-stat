#include "statistic.h"

#include <QApplication>
//#include <QTextCodec>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

//	QTextCodec::setCodecForTr(QTextCodec::codecForName("windows-1251"));

	Statistic w;
	w.show();

	return a.exec();
}
