#ifndef STATISTIC_H
#define STATISTIC_H

#include <QWidget>
#include <QFtp>
#include <QFile>
#include <QProgressDialog>
#include <QTcpSocket>

#include "ui_statistic.h"
#include "CXSettingsDialog.h"

#include "qttelnet.h"

#define FTP_USER      "ftp"
#define FTP_PASSWORD  "ftp"
#define FTP_PORT      21
#define TELNET_PORT   23
#define TELNET_ARCHIVE_PATH "/pub/updates/"
#define TELNET_SCRIPT       "/CNC/backup_log.xml.sh"

//! Класс основной формы приложения.
class Statistic : public QWidget
{
	Q_OBJECT

public:
	Statistic(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Statistic();

private slots:
  void telnetLoginRequired();
  void telnetLoginFailed();
  void telnetConnectionError(QAbstractSocket::SocketError error);
  void telnetMessage(const QString &data);


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
//	/*!
//		Слот на доступность данных для сохранения.
//	*/
//	void onReadyRead();

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

	QtTelnet* tlClient;
	QString user;
	QString password;
	QString host;

	bool start_arch;

	int waitTimer;
	bool mIsArchiveAccept;
};

#endif // STATISTIC_H
