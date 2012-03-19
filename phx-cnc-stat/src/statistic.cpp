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

Statistic::Statistic(QWidget *parent, Qt::WFlags flags) : QWidget(parent, flags)
{
	Engine::removeOldDirs();

	ui.setupUi(this);

	mSettingsDialog = new CXSettingsDialog(this);

	mFtp = new QFtp(this);
	mLoadFile = NULL;
	mProgressBar = NULL;
	waitTimer = -1;
	mIsArchiveAccept = false;

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

	connect(mFtp, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

#ifdef QT_NO_DEBUG
	ui.mErrorGroupBox->hide();
#endif
}

Statistic::~Statistic()
{
	CXSettings::inst()->save(QApplication::applicationDirPath() + "/settings.xml");
}

void Statistic::onArchiveLoad()
{
	QString fileName = QFileDialog::getOpenFileName(NULL, trUtf8("Загрузка архива"), QString()/*, "HTML (*.html)"*/);

	if (fileName.isEmpty()) return;

	QDir logsDir(QApplication::applicationDirPath() + "/" + LOG_PATH);
	if (!logsDir.exists())
	{
		QDir(QApplication::applicationDirPath()).mkpath(LOG_PATH);
	}

	QString copyFileName = QApplication::applicationDirPath() + "/" + ARCHIVE_PATH + "/" + QFileInfo(fileName).fileName();

	if(QFile::exists(copyFileName))
		QFile::remove(copyFileName);

	QFile::copy(fileName, copyFileName);

	if (unCompress(copyFileName))
	{
		QFile(copyFileName).remove();

		QMessageBox::information(this, trUtf8("Внимание"), trUtf8("Архив успешно распакован"));
	}
}

void Statistic::onFTPLoad()
{
	ui.mFTPButton->setEnabled(false);

	CXSettings* settings = CXSettings::inst();

	QString createArchiveScript = settings->value(E_CreateArchiveScript).toString();
	createArchiveScript.replace("[APP_PATH]", QApplication::applicationDirPath());

	//Запус скрипта создания архива.
	if (QProcess::execute(createArchiveScript) != 0)
	{
		QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Не отработал скрипт создания архива:\n[%1]").arg(settings->value(E_CreateArchiveScript).toString()));
		return;
	}

	//Получение архива с FTP-сервера.
	QString host = settings->value(E_FTPHost).toString();
	int port = settings->value(E_FTPPort).toInt();
	QString user = CXSettings::inst()->value(E_FTPUser).toString();
	QString password = CXSettings::inst()->value(E_FTPPassword).toString();

	mFtp->connectToHost(host, port);
	if (mFtp->error() != QFtp::NoError)
	{
		onFtpError(trUtf8("Не удалось подключиться к FTP-серверу:\n%1").arg(mFtp->errorString()));

		return;
	}
}

void Statistic::onReport()
{
	CXSettings* settings = CXSettings::inst();

	settings->setValue(E_StartDate, QDateTime(ui.mStartDateEdit->date(), ui.mStartTimeEdit->time()));
	settings->setValue(E_EndDate, QDateTime(ui.mEndDateEdit->date(), ui.mEndTimeEdit->time()));

	QStringList xmlFiles;

	QDir logsDir(QApplication::applicationDirPath() + "/" + LOG_PATH);

	QString tempFileName;
	QStringList dirs = logsDir.entryList(QDir::Dirs);
	for (int i = 0; i < dirs.count(); ++i)
	{
		tempFileName = logsDir.path() + "/" + dirs.at(i) + "/report.xml";

		if (QFile::exists(tempFileName)) xmlFiles.append(tempFileName);
	}

	QList <SXReportError> errorList = Engine::generateReport(xmlFiles, settings->value(E_StartDate).toDateTime(), settings->value(E_EndDate).toDateTime());

#ifndef QT_NO_DEBUG
	ui.mErrorTree->clear();

	QTreeWidgetItem* newItem = NULL;
	QTreeWidgetItem* childItem = NULL;
	QString lastFilename = "";

	for (int i = 0; i < errorList.count(); ++i)
	{
		const SXReportError& curErrorData = errorList.at(i);

		if ((newItem == NULL) || ((newItem != NULL) && (lastFilename != curErrorData.mFileName)))
		{
			newItem = new QTreeWidgetItem(ui.mErrorTree);
			lastFilename = curErrorData.mFileName;
			newItem->setText(0, lastFilename);
		}

		childItem = new QTreeWidgetItem(newItem);
		childItem->setForeground(0, Qt::red);
		childItem->setText(0, curErrorData.mError);

		ui.mErrorTree->addTopLevelItem(newItem);
	}
#else
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
#endif

	ui.mReportView->setHtml(Engine::getReportText());
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
			waitTimer = startTimer(1000);
			break;
		}
		default:
			break;
	}
}

void Statistic::onFtpCommandFinish(int id, bool aIsError)
{
	qDebug("commE: %d", mFtp->currentCommand());

	switch (mFtp->currentCommand())
	{
		case QFtp::ConnectToHost:
		{
			if (aIsError) onFtpError(trUtf8("Не удалось подключиться к FTP-серверу. Сервер недоступен.\n[%1]").arg(mFtp->errorString()));
			else
			{
				killTimer(waitTimer);
				waitTimer = -1;

				QString user = CXSettings::inst()->value(E_FTPUser).toString();
				QString password = CXSettings::inst()->value(E_FTPPassword).toString();

				mFtp->login(user, password);
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

			QString dir = CXSettings::inst()->value(E_FTPDir).toString();
			if (dir != "")
			{
				mFtp->cd(dir);
				if (mFtp->error() != QFtp::NoError)
				{
					QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Не удалось подключиться к FTP-серверу:\n%1").arg(mFtp->errorString()));
					mFtp->close();
					return;
				}

			}
			break;
		}
		case QFtp::Cd:
		{
			if (aIsError) onFtpError(trUtf8("Не удалось подключиться к FTP-серверу. Неверная пара логин/пароль.\n[%1]").arg(mFtp->errorString()));

			mIsArchiveAccept = false;
			mFtp->list();

			break;
		}
		case QFtp::List:
		{
			if (aIsError) onFtpError(trUtf8("Не удалось выполнить команду LIST.\n[%1]").arg(mFtp->errorString()));
			if (!mIsArchiveAccept) onFtpError(trUtf8("Не удалось получить архив \"%1\" с сервера.").arg(CXSettings::inst()->value(E_ArchiveName).toString()));

			break;
		}
		default:
			break;

	}

	if (mFtp->currentCommand() == QFtp::Get)
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
	}
}

void Statistic::onListInfo(const QUrlInfo& aInfo)
{
	if (aInfo.isFile() && aInfo.name() == CXSettings::inst()->value(E_ArchiveName).toString())
	{
		mIsArchiveAccept = true;

		if (mProgressBar == NULL)
		{
			mProgressBar = new QProgressDialog(this);

			QProgressBar* progressBar = new QProgressBar(mProgressBar);
			progressBar->setAlignment(Qt::AlignCenter);
			progressBar->setRange(0, aInfo.size());
			progressBar->setValue(0);

			mProgressBar->setBar(progressBar);
			mProgressBar->setCancelButton(NULL);

			mProgressBar->setLabelText(trUtf8("Загрузка архива"));
		}
		else
		{
			mProgressBar->setRange(0, aInfo.size());
			mProgressBar->setValue(0);
		}
//		mProgressBar->setModal(true);
		mProgressBar->show();

		QString archiveName = CXSettings::inst()->value(E_ArchiveName).toString();

		mLoadFile = new QFile(QApplication::applicationDirPath() + "/" + ARCHIVE_PATH + "/" + archiveName);
		if (!mLoadFile->open(QIODevice::WriteOnly))
		{
			QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Не удалось создать файл:\n%1").arg(mLoadFile->fileName()));
			mFtp->close();
			return;
		}

		mFtp->get(archiveName);
	}
}

void Statistic::onReadyRead()
{
	if (mProgressBar != NULL)
	{
		mProgressBar->setValue(mProgressBar->value() + mFtp->bytesAvailable());
		mLoadFile->write(mFtp->readAll());
	}
}

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
    QString unpack = "unpack.bat";
    if (QProcess::execute(unpack) != 0)
	{
        QMessageBox::critical(this, trUtf8("Ошибка")
                              , trUtf8("Не удалось распаковать архив  "));
		return false;
	}
	return true;
}

void Statistic::onFtpError(const QString& aErrorText)
{
	mFtp->close();
	ui.mFTPButton->setEnabled(true);

	if (waitTimer != -1)
	{
		killTimer(waitTimer);
		waitTimer = -1;
	}

	QMessageBox::information(this, trUtf8("Ошибка"), aErrorText);
}
