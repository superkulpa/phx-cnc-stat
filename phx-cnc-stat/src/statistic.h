#ifndef STATISTIC_H
#define STATISTIC_H

#include <QWidget>
#include <QFtp>
#include <QFile>
#include <QProgressDialog>

#include "ui_statistic.h"
#include "CXSettingsDialog.h"

//! Класс основной формы приложения.
class Statistic : public QWidget
{
	Q_OBJECT

public:
	Statistic(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Statistic();

private slots:
	/*!
		Слот загрузки и распаковки архива из файла.
	*/
	void onArchiveLoad();
	/*!
		Слот инициализации загрузки архива по FTP.
	*/
	void onFTPLoad();
	/*!
		Слот на генерацию отчета.
	*/
	void onReport();
	/*!
		Слот на сохранение отчетка.
	*/
	void onSaveReport();

	/*!
		Слот на старт команды FTP.
	*/
	void onCommandStarted(int id);
	/*!
		Слот на завершение команды FTP.
	*/
	void onFtpCommandFinish(int, bool aIsError);
	/*!
		Слот на получение данных от команы list.
	*/
	void onListInfo(const QUrlInfo& aInfo);
	/*!
		Слот на доступность данных для сохранения.
	*/
	void onReadyRead();

protected:
	void timerEvent(QTimerEvent* e);

private:
	/*!
		Функция распаковки файла архива.
		\return Успешность распаковки.
	*/
	bool unCompress(const QString& aArchiveName);
	/*!
		Функция обработки ошибки подключения к FTP-серверу
	*/
	void onFtpError(const QString& aErrorText);

private:
	Ui::StatisticClass ui;

	CXSettingsDialog* mSettingsDialog;

	QFtp* mFtp;
	QFile* mLoadFile;

	QProgressDialog* mProgressBar;
	int waitTimer;
	bool mIsArchiveAccept;
};

#endif // STATISTIC_H
