#include "statistic.h"


#include <QDomDocument>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QProgressBar>

#include "engine/engine.h"

#define VERSION  "1.0"

Statistic::Statistic(QWidget *parent, Qt::WFlags flags) : QWidget(parent, flags)
{
	Engine::removeOldDirs();

	ui.setupUi(this);
	QString title = trUtf8("Статистика v") + VERSION;
	title += trUtf8(", сборка от "); title +=  __DATE__;
	this->setWindowTitle(title);

	mSettingsDialog = new CXSettingsDialog(this);

	mFtp = new QFtp(this);
	mLoadFile = NULL;
	waitTimer = -1;
	mIsArchiveAccept = false;
	start_arch = false;

	CXSettings* settings = CXSettings::inst();

	if (settings->value(E_StartDate).toDateTime().isValid())
	{
		ui.mStartDateEdit->setDateTime(settings->value(E_StartDate).toDateTime());
		ui.mStartTimeEdit->setTime(settings->value(E_StartDate).toDateTime().time());
	}

	if (settings->value(E_EndDate).toDateTime().isValid())
	{
		ui.mEndDateEdit->setDateTime(settings->value(E_EndDate).toDateTime());
		ui.mEndTimeEdit->setTime(settings->value(E_EndDate).toDateTime().time());
	}

	connect(ui.mArchiveButton,	SIGNAL(clicked()), this, SLOT(onArchiveLoad()));
	connect(ui.mFTPButton,		SIGNAL(clicked()), this, SLOT(onFTPLoad()));
	connect(ui.mReportButton,	SIGNAL(clicked()), this, SLOT(onReport()));
	connect(ui.mSaveReportButton,	SIGNAL(clicked()), this, SLOT(onSaveReport()));
	connect(ui.mSettingsButton,	SIGNAL(clicked()), mSettingsDialog, SLOT(exec()));

	connect(mFtp, SIGNAL(commandStarted(int)), this, SLOT(onCommandStarted(int)));
	connect(mFtp, SIGNAL(commandFinished(int,bool)), this, SLOT(onFtpCommandFinish(int,bool)));
	connect(mFtp, SIGNAL(listInfo(const QUrlInfo&)), this, SLOT(onListInfo(const QUrlInfo&)));
//	connect(mFtp, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

	tlClient = new QtTelnet(this);

	connect(tlClient, SIGNAL(loginRequired()), this, SLOT(telnetLoginRequired()));
  connect(tlClient, SIGNAL(loginFailed()), this, SLOT(telnetLoginFailed()));
  connect(tlClient, SIGNAL(connectionError(QAbstractSocket::SocketError)),
      this, SLOT(telnetConnectionError(QAbstractSocket::SocketError)));
  connect(tlClient, SIGNAL(message(const QString &)),this, SLOT(telnetMessage(const QString &)));

//#ifdef QT_NO_DEBUG
//	ui.mErrorGroupBox->hide();
//#endif
}

Statistic::~Statistic()
{
#ifdef DEBUG_VER
  QString tmp = "D:/CNC_stat";
#else
  QString tmp = QApplication::applicationDirPath();
#endif
	CXSettings::inst()->save(tmp + "/settings.xml");
	tlClient->close();
}

void Statistic::onArchiveLoad()
{
	QString fileName = QFileDialog::getOpenFileName(NULL, trUtf8("Загрузка архива"), QString()/*, "HTML (*.html)"*/);

	if (fileName.isEmpty()) return;
#ifdef DEBUG_VER
  QString tmp = "D:/CNC_stat";
#else
  QString tmp = QApplication::applicationDirPath();
#endif
	QDir logsDir(tmp + "/" + LOG_PATH);
	if (!logsDir.exists())
	{
		QDir(tmp).mkpath(LOG_PATH);
	}

	QString copyFileName = tmp + "/" + ARCHIVE_PATH + "/" + QFileInfo(fileName).fileName();

	if(QFile::exists(copyFileName))
		QFile::remove(copyFileName);

	QFile::copy(fileName, copyFileName);

	if (unCompress(copyFileName))
	{
		QFile(copyFileName).remove();
		QMessageBox::information(this, trUtf8("Внимание"), trUtf8("Архив успешно распакован"));
		ui.curOperationLabel->setText(trUtf8("Нет текущих операций"));
		setCursor(Qt::ArrowCursor);
	}
}

void Statistic::onFTPLoad()
{
	ui.mFTPButton->setEnabled(false);

	CXSettings* settings = CXSettings::inst();
	//считываем настройки
	host = settings->value(E_FTPHost).toString();
  user = CXSettings::inst()->value(E_TelnetUser).toString();
  password = CXSettings::inst()->value(E_TelnetPassword).toString();

  //приходится каждый раз убивать, так как не дает переподключатся
  if(tlClient != NULL){
    delete tlClient;
    tlClient = NULL;
  }
  ui.curOperationLabel->setText(trUtf8("Соединение с сервером..."));
  setCursor(Qt::WaitCursor);
  tlClient = new QtTelnet(this);

  connect(tlClient, SIGNAL(loginRequired()), this, SLOT(telnetLoginRequired()));
  connect(tlClient, SIGNAL(loginFailed()), this, SLOT(telnetLoginFailed()));
  connect(tlClient, SIGNAL(connectionError(QAbstractSocket::SocketError)),
      this, SLOT(telnetConnectionError(QAbstractSocket::SocketError)));
  connect(tlClient, SIGNAL(message(const QString &)),this, SLOT(telnetMessage(const QString &)));

  //соединяемся с telnet клиентом
  if(!tlClient->connectToHost(host, TELNET_PORT)){
    onFtpError(trUtf8("Не удалось подключиться к Telnet-серверу. Сервер недоступен.\n[%1]").arg(host));
  };
}

void Statistic::telnetLoginRequired(){
  qDebug() << "loginRequired";
  QString tmp = user + "\n" + password;
  tlClient->sendData(tmp);
};

void Statistic::telnetLoginFailed(){
  qDebug() << "loginFailed";
};

void Statistic::telnetConnectionError(QAbstractSocket::SocketError error){
  qDebug() << "ConnectError";
  onFtpError(trUtf8("Не удалось подключиться к Telnet-серверу, соединение оборвалось"));
};

void Statistic::telnetMessage(const QString &data){
  qDebug() << "message";
  if(((((data.indexOf("$",0) != -1) || (data.indexOf("#",0) != -1)) && (data.size() < 10)) || (data.indexOf("Welcome",0) != -1)) && (!start_arch)){
    ui.curOperationLabel->setText(trUtf8("Архивирование данных..."));
    setCursor(Qt::WaitCursor);
    tlClient->sendData(TELNET_SCRIPT);
    start_arch = true;
  }else if((data.indexOf("complete",0) != -1) && (start_arch)){
    start_arch = false;
    tlClient->close();
    ui.curOperationLabel->setText(trUtf8("Инициализация ftp-клиента"));
    setCursor(Qt::WaitCursor);
    mFtp->connectToHost(host, FTP_PORT);
  }else if((data.indexOf("fault",0) != -1) && (start_arch)){
    start_arch = false;
    int indx = data.indexOf("fault",0);
    QString error = ""; int tmp = 0;
    while(data[indx + 7 + tmp] != '\r'){
      error += data[indx + 7 + tmp++];
    };
    onFtpError(trUtf8("Ошибка архивирования: \n [%1]").arg(error));
  }else if((data.indexOf("incorrect",0) != -1)){
    //ошибка мы не подключились
    start_arch = false;
    onFtpError(trUtf8("Не удалось подключиться к Telnet-серверу. Неправильное имя пользователя или пароль"));
  };
};

void Statistic::onReport()
{
  ui.curOperationLabel->setText(trUtf8("Анализ отчета..."));
  setCursor(Qt::WaitCursor);
	CXSettings* settings = CXSettings::inst();

	settings->setValue(E_StartDate, QDateTime(ui.mStartDateEdit->date(), ui.mStartTimeEdit->time()));
	settings->setValue(E_EndDate, QDateTime(ui.mEndDateEdit->date(), ui.mEndTimeEdit->time()));

	QStringList xmlFiles;
#ifdef DEBUG_VER
  QString tmp = "D:/CNC_stat";
#else
  QString tmp = QApplication::applicationDirPath();
#endif
	QDir logsDir(tmp + "/" + LOG_PATH);
	//QDir logsDir(QApplication::applicationDirPath() + "/" + LOG_PATH);
	QString tempFileName;
	QStringList dirs = logsDir.entryList(QDir::Dirs);
	for (int i = 0; i < dirs.count(); ++i)
	{
		tempFileName = logsDir.path() + "/" + dirs.at(i) + "/report.xml";

		if (QFile::exists(tempFileName)) xmlFiles.append(tempFileName);
	}

	QList <SXReportError> errorList = Engine::generateReport(xmlFiles, settings->value(E_StartDate).toDateTime(), settings->value(E_EndDate).toDateTime());

//#ifndef QT_NO_DEBUG
//	ui.mErrorTree->clear();
//
//	QTreeWidgetItem* newItem = NULL;
//	QTreeWidgetItem* childItem = NULL;
//	QString lastFilename = "";
//
//	for (int i = 0; i < errorList.count(); ++i)
//	{
//		const SXReportError& curErrorData = errorList.at(i);
//
//		if ((newItem == NULL) || ((newItem != NULL) && (lastFilename != curErrorData.mFileName)))
//		{
//			newItem = new QTreeWidgetItem(ui.mErrorTree);
//			lastFilename = curErrorData.mFileName;
//			newItem->setText(0, lastFilename);
//		}
//
//		childItem = new QTreeWidgetItem(newItem);
//		childItem->setForeground(0, Qt::red);
//		childItem->setText(0, curErrorData.mError);
//
//		ui.mErrorTree->addTopLevelItem(newItem);
//	}
//#else
	QFile errorLog("errors.log");
	errorLog.open(QIODevice::WriteOnly);

	QTextStream textStream(&errorLog);
	textStream.setCodec(QTextCodec::codecForName("UTF-8"));

	for (int i = 0; i < errorList.count(); ++i)
	{
		const SXReportError& curErrorData = errorList.at(i);

		textStream << curErrorData.mFileName << "\t-\t";
		textStream << curErrorData.mError << "\n";
	}
//#endif

	ui.mReportView->setHtml(Engine::getReportText());
	ui.curOperationLabel->setText(trUtf8("Нет текущих операций"));
	setCursor(Qt::ArrowCursor);
}

void Statistic::onSaveReport()
{
	Engine::saveReport();
}

void Statistic::onCommandStarted(int id)
{
	qDebug("commS: %d", mFtp->currentCommand());

	switch (mFtp->currentCommand())
	{
		case QFtp::ConnectToHost:
		{
			waitTimer = startTimer(10000);
			break;
		}
		default:
			break;
	}
}

void Statistic::onFtpCommandFinish(int id, bool aIsError)
{
	qDebug("commE: %d", mFtp->currentCommand());
	if(aIsError) onFtpError(trUtf8("\n[%1]").arg(mFtp->errorString()));
	switch (mFtp->currentCommand())
	{
		case QFtp::ConnectToHost:
		{
			if (aIsError) onFtpError(trUtf8("Не удалось подключиться к FTP-серверу. Сервер недоступен.\n[%1]").arg(mFtp->errorString()));
			else
			{
				killTimer(waitTimer);
				waitTimer = -1;
				mFtp->login(FTP_USER, FTP_PASSWORD);
				if (mFtp->error() != QFtp::NoError)
				{
					onFtpError(trUtf8("Не удалось подключиться к FTP-серверу:\n[%1]").arg(mFtp->errorString()));

					return;
				}

				waitTimer = startTimer(10000);
			}
			break;
		}
		case QFtp::Login:
		{
			killTimer(waitTimer);
			waitTimer = -1;

			if (aIsError)
			{
				onFtpError(trUtf8("Не удалось подключиться к FTP-серверу. Неверная пара логин/пароль.\n[%1]").arg(mFtp->errorString()));
				break;
			}
			ui.curOperationLabel->setText(trUtf8("Копирование архива..."));
			setCursor(Qt::WaitCursor);
      mFtp->cd(TELNET_ARCHIVE_PATH);
      if (mFtp->error() != QFtp::NoError)
      {
        QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Не удалось подключиться к FTP-серверу:\n%1").arg(mFtp->errorString()));
        mFtp->close();
        return;
      }
			break;
		}
		case QFtp::Cd:
		{
			if (aIsError) onFtpError(trUtf8("Не удалось подключиться к FTP-серверу. Неверная пара логин/пароль.\n[%1]").arg(mFtp->errorString()));
#ifdef DEBUG_VER
      QString tmp = "D:/CNC_stat";
#else
      QString tmp = QApplication::applicationDirPath();
#endif
			mLoadFile = new QFile(tmp + "/" + ARCHIVE_PATH + "/" + ARCHIVE_NAME);
      //mLoadFile = new QFile(QApplication::applicationDirPath() + "/" + ARCHIVE_PATH + "/" + ARCHIVE_NAME);
      if (!mLoadFile->open(QIODevice::WriteOnly))
      {
        QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Не удалось создать файл:\n%1").arg(mLoadFile->fileName()));
        mFtp->close();
        return;
      }
      tmp = TELNET_ARCHIVE_PATH;
      mFtp->get(tmp + ARCHIVE_NAME, mLoadFile);
      waitTimer = startTimer(1000);
			break;
		}
		case QFtp::List:
		{
			if (aIsError) onFtpError(trUtf8("Не удалось выполнить команду LIST.\n[%1]").arg(mFtp->errorString()));
			if (!mIsArchiveAccept) onFtpError("Не удалось получить архив с сервера.");

			break;
		}
		case QFtp::Get:
    {
      mLoadFile->close();
      mFtp->close();
#ifdef DEBUG_VER
      QString tmp = "D:/CNC_stat";
#else
      QString tmp = QApplication::applicationDirPath();
#endif
      tmp = tmp + "/" + ARCHIVE_PATH + "/" + ARCHIVE_NAME;
      ui.mFTPButton->setEnabled(true);
      if(unCompress(tmp)){
        ui.curOperationLabel->setText(trUtf8("Нет текущих операций"));
        setCursor(Qt::ArrowCursor);
        QFile(tmp).remove();
        QMessageBox::information(this, trUtf8("Внимание"), trUtf8("Архив успешно распакован"));
        return;
      }
      break;
    }
    default:
    break;

	}

	/*if (mFtp->currentCommand() == QFtp::Get)
	{
		if (mProgressBar != NULL) mProgressBar->close();

		mLoadFile->close();
		mFtp->close();

		CXSettings* settings = CXSettings::inst();

		QString archiverPath = settings->value(E_ArchiverPath).toString();
		if (archiverPath.isEmpty())
		{
			onFtpError(trUtf8("Заполните поле с путем к архиватору!"));
			return;
		}

		QString archiveFileName = QApplication::applicationDirPath() + "/" + ARCHIVE_PATH + "/" + settings->value(E_ArchiveName).toString();

		if (!unCompress(archiveFileName))
		{
			ui.mFTPButton->setEnabled(true);

			return;
		}

//		QFile(archiveFileName).remove();

		QMessageBox::information(this, trUtf8("Внимание"), trUtf8("Архив успешно скачан и распакован"));

		ui.mFTPButton->setEnabled(true);
	}*/
}

void Statistic::onListInfo(const QUrlInfo& aInfo)
{
//	if (true)//(aInfo.isFile() && aInfo.name() == CXSettings::inst()->value(E_ArchiveName).toString())
//	{
//		mIsArchiveAccept = true;
//
//		if (mProgressBar == NULL)
//		{
//			mProgressBar = new QProgressDialog(this);
//
//			QProgressBar* progressBar = new QProgressBar(mProgressBar);
//			progressBar->setAlignment(Qt::AlignCenter);
//			progressBar->setRange(0, aInfo.size());
//			progressBar->setValue(0);
//
//			mProgressBar->setBar(progressBar);
//			mProgressBar->setCancelButton(NULL);
//
//			mProgressBar->setLabelText(trUtf8("Загрузка архива"));
//		}
//		else
//		{
//			mProgressBar->setRange(0, aInfo.size());
//			mProgressBar->setValue(0);
//		}
////		mProgressBar->setModal(true);
//		mProgressBar->show();
//
//		QString archiveName = CXSettings::inst()->value(E_ArchiveName).toString();
//		QString tmp = "D:/CNC_stat/";
//    mLoadFile = new QFile(tmp + ARCHIVE_PATH + "/" + archiveName);
//    //new QFile(QApplication::applicationDirPath() + "/" + ARCHIVE_PATH + "/" + archiveName);
//		//mLoadFile = new QFile(QApplication::applicationDirPath() + "/" + ARCHIVE_PATH + "/" + archiveName);
//		if (!mLoadFile->open(QIODevice::WriteOnly))
//		{
//			QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Не удалось создать файл:\n%1").arg(mLoadFile->fileName()));
//			mFtp->close();
//			return;
//		}
//
//		mFtp->get(archiveName, mLoadFile);
//		waitTimer = startTimer(1000);
//	}
}

//void Statistic::onReadyRead()
//{
//	if (mProgressBar != NULL)
//	{
//		mProgressBar->setValue(mProgressBar->value() + mFtp->bytesAvailable());
//		mLoadFile->write(mFtp->readAll());
//	}
//}

void Statistic::timerEvent(QTimerEvent* e)
{
	qDebug("timer: %d", mFtp->currentCommand());
	if (e->timerId() == waitTimer)
	{
		killTimer(waitTimer);
		waitTimer = -1;

		switch (mFtp->currentCommand())
		{
			case QFtp::ConnectToHost:
			{
				onFtpError(trUtf8("Не удалось подключиться к FTP-серверу. Сервер недоступен."));

				break;
			}
			case QFtp::Login:
			{
				onFtpError(trUtf8("Не удалось подключиться к FTP-серверу. Неверная пара логин/пароль."));

				break;
			}
			default:
      break;

		}
	}
}

bool Statistic::unCompress(const QString& aArchiveName)
{
  ui.curOperationLabel->setText(trUtf8("Разархивирование данных..."));
  setCursor(Qt::WaitCursor);
  QString unpack = "unpack.bat";
  if (QProcess::execute(unpack) != 0)
	{
    QMessageBox::critical(this, trUtf8("Ошибка")
                          , trUtf8("Не удалось распаковать архив"));
//    if(mFtp->state() != QFtp::Unconnected)
//      mFtp->close();
    start_arch = false;
    ui.curOperationLabel->setText(trUtf8("Нет текущих операций"));
    setCursor(Qt::ArrowCursor);

		return false;
	}
	return true;
}

void Statistic::onFtpError(const QString& aErrorText)
{
  if(mFtp->state() != QFtp::Unconnected)
    mFtp->close();
  start_arch = false;
  ui.curOperationLabel->setText(trUtf8("Нет текущих операций"));
  setCursor(Qt::ArrowCursor);
  tlClient->close();

	ui.mFTPButton->setEnabled(true);

	if (waitTimer != -1)
	{
		killTimer(waitTimer);
		waitTimer = -1;
	}

	QMessageBox::information(this, trUtf8("Ошибка"), aErrorText);
}
