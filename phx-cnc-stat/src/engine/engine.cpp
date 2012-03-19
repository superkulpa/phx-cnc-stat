#include "engine.h"

#include <QFile>
#include <QApplication>
#include <QTextStream>
#include <QTextCodec>
#include <QFileDialog>

#include "../CXSettings.h"
#include "../CXSectionDialog.h"
#include "../SXParamData.h"

QString Engine::mReportText;

QList <SXReportError> Engine::generateReport(const QStringList& aFileList, const QDateTime& aStartDate, const QDateTime& aEndDate)
{
	QMap <QString, SXParamData> paramData;

	QList <SXReportError> errorList = fillParamsData(paramData);

	SXReportError curError;

	CXSettings* settings = CXSettings::inst();

	QFile xmlFile;
	QDomDocument domDocument;
	QDateTime curDate;
	QDomElement rootElement, sectionElement, paramElement;

	QFile headerHTML(settings->value(E_HeaderReport).toString());

	mReportText = QObject::trUtf8("<html><body>С {Start Date} по {End Date}<pre>");

	if (headerHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&headerHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		mReportText = textStream.readAll();
	}

	mReportText = mReportText.replace("{Start Date}",	aStartDate.toString("dd.MM.yyyy hh:mm:ss"));
	mReportText = mReportText.replace("{End Date}",		aEndDate.toString("dd.MM.yyyy hh:mm:ss"));

/**/
	QFile itemHTML(settings->value(E_SectionReport).toString());

	QString sectionTemplate = QObject::trUtf8("{Section Descr}\n{Params}");

	if (itemHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&itemHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		sectionTemplate = textStream.readAll();
	}

	itemHTML.close();
	itemHTML.setFileName(settings->value(E_ParamReport).toString());

	QString paramTemplate = QObject::trUtf8("	{Param Descr}	{Param Value}\n");

	if (itemHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&itemHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		paramTemplate = textStream.readAll();
	}
/**/
	QList <SXParamData> reportData;

	//Обработка xml-файла и заполнение данных для отчета в список mReportData.
	for (int i = 0; i < aFileList.count(); ++i)
	{
		const QString& curFileName = aFileList.at(i);
		curError.mFileName = curFileName;

		xmlFile.setFileName(curFileName);
		if (!xmlFile.open(QIODevice::ReadOnly))
		{
			curError.mError = QObject::trUtf8("Не удалось открыть файл [%1]").arg(curFileName);

			errorList.append(curError);

			continue;
		}

		QString errorMsg;
		int errorLine, errorColumn;

		if (!domDocument.setContent(&xmlFile, &errorMsg, &errorLine, &errorColumn))
		{
			xmlFile.close();

			curError.mError = QObject::trUtf8("Не корректный xml: \"%1\" строка %2 столбец %3").arg(errorMsg).arg(errorLine).arg(errorColumn);

			errorList.append(curError);

			continue;
		}

		xmlFile.close();

		rootElement = domDocument.documentElement();
		if (rootElement.tagName() != "Base")
		{
			curError.mError = QObject::trUtf8("Не корректный xml: корневой элемент отличен от Base");

			errorList.append(curError);

			continue;
		}

		curDate = QDateTime::fromString(rootElement.attribute("start"), "dd.MM.yyyy hh:mm:ss");

		//если дата лог-файла не подходит под интервал или не валидная - пропустить.
		if (!curDate.isValid())
		{
			curError.mError = QObject::trUtf8("Не корректная или отсутствующая дата в элементе Base, атрибут date");

			errorList.append(curError);

			continue;
		}

		if (/*curDate < aStartDate || */curDate > aEndDate) continue;

		qreal ticToSec = rootElement.attribute("tic_to_sec").toFloat();
		if (ticToSec == 0)
		{
			curError.mError = QObject::trUtf8("Значение атрибута tic_to_sec в элементе Base не может быть нулевым.");

			errorList.append(curError);

			continue;
		}

		sectionElement = rootElement.firstChildElement("Logs");

		QDateTime tempDateTime;
		SXParamData curData;
		SXParamData tempData;

		while (sectionElement.isElement())
		{
			paramElement = sectionElement.firstChildElement();

			while (paramElement.isElement())
			{
				if (!paramElement.hasAttribute("time") || paramElement.attribute("time").toFloat() <= 0)
				{
					paramElement = paramElement.nextSiblingElement();
					continue;
				}

				tempDateTime = curDate.addSecs(paramElement.attribute("time").toFloat() / ticToSec);

				if (tempDateTime < aStartDate || tempDateTime > aEndDate)
				{
					paramElement = paramElement.nextSiblingElement();
					continue;
				}

				curData.mParamName = paramElement.tagName();
				curData.mValue = paramElement.attribute("value").toInt();

				if (!paramData.contains(curData.mParamName))
				{
					curError.mError = QObject::trUtf8("Не удалось найти описание для параметра [%1]").arg(curData.mParamName);

					errorList.append(curError);
				}
				
				tempData = paramData.value(curData.mParamName);

				curData.mParamDescr = tempData.mParamDescr;
				curData.mSectionName = tempData.mSectionName;
				curData.mSectionDescr = tempData.mSectionDescr;
				curData.mType = tempData.mType;

				//если уже есть такой параметр - суммировать значение.
				if (reportData.contains(curData))
				{
					reportData[reportData.indexOf(curData)].mValue += curData.mValue;
				}
				else //иначе - добавить.
				{
					reportData.append(curData);
				}

				paramElement = paramElement.nextSiblingElement();
			}

			sectionElement = sectionElement.nextSiblingElement("Section");
		}
	}
/**/
	if (reportData.isEmpty()) mReportText.append(QObject::trUtf8("Нет накопленных данных"));
	else
	{
		CXSectionDialog sectionDialog(reportData);
		sectionDialog.exec();

		QStringList ignoredSectionsList = CXSettings::inst()->value(E_IgnoredSections).toStringList();

		QString sectionName;
		QString sectionText, paramText;

		//Заполнение данных в отчет из списка mReportData.
		while (!reportData.isEmpty())
		{
			const SXParamData& curSection = reportData.first();
			sectionName = curSection.mSectionName;

			if (ignoredSectionsList.contains(sectionName))
			{
				reportData.removeFirst();

				continue;
			}

			paramText.clear();
			sectionText = sectionTemplate;
			sectionText = sectionText.replace("{Section}", curSection.mSectionName).replace("{Section Descr}", curSection.mSectionDescr);

			for (int i = 0; i < reportData.count(); ++i)
			{
				const SXParamData& curData = reportData.at(i);

				if (curData.mSectionName == sectionName)
				{
					paramText.append(QString(paramTemplate).replace("{Param Name}", curData.mParamName).replace("{Param Descr}", curData.mParamDescr).replace("{Param Value}", curData.getValue()));
					reportData.removeAt(i);
					--i;
				}
			}

			sectionText = sectionText.replace("{Params}", paramText);
			mReportText.append(sectionText);
		}
	}
/**/

	QFile footerHTML(settings->value(E_FooterReport).toString());

	if (footerHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&footerHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		mReportText.append(textStream.readAll());
	}
	else mReportText.append("</pre></body></html>");

	return errorList;
}

QString Engine::getReportText()
{
	return mReportText;
}

void Engine::saveReport()
{
	QString fileName = QFileDialog::getSaveFileName(NULL, QObject::trUtf8("Сохранение отчета"), QString(), "HTML (*.html)");

	if (fileName.isEmpty()) return;

	QFile file(fileName);

	if (!file.open(QIODevice::WriteOnly)) return;

	QTextStream textStream(&file);
	textStream.setCodec(QTextCodec::codecForName("UTF-8"));
	textStream << mReportText;

	file.close();
}

void Engine::removeOldDirs()
{
	int logDays = CXSettings::inst()->value(E_LogPeriod).toInt();

	//если журнал хранится ограниченное количество дней.
	if (logDays > 0)
	{
		QDir logsDir(QApplication::applicationDirPath() + "/" + LOG_PATH);

		//получение списка всех папок в логе.
		QStringList dirs = logsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		QDateTime curDate = QDateTime::currentDateTime();
		QDateTime tempDate;
//		QStringList fileList;
		QString curDir;

		for (int i = 0; i < dirs.count(); ++i)
		{
			curDir = dirs.at(i);
			tempDate = QDateTime::fromString(curDir, "logyyyy_MM_dd__hh_mm_ss");

			//если дата папки лога валидная и время ее хранения вышло - удалить.
			if (tempDate.isValid() && curDate > tempDate && tempDate.daysTo(curDate) > logDays)
			{
/*
				fileList = QDir(logsDir.path() + "/" + curDir).entryList(QDir::Files);

				//удаление файлов из папки.
				for (int j = 0; j < fileList.count(); j++)
				{
					QFile::remove(logsDir.path() + "/" + curDir + "/" + fileList.at(i));
				}
*/
				QFile::remove(logsDir.path() + "/" + curDir + "/report.xml");

				logsDir.rmdir(curDir);
			}
		}
	}
}

QList <SXReportError> Engine::fillParamsData(QMap <QString, SXParamData>& aParamsData)
{
	QList <SXReportError> errorList;
	SXReportError curError;

	QString fileName = QApplication::applicationDirPath() + "/descrips.xml";

	curError.mFileName = fileName;

	QFile xmlFile(fileName);
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		curError.mError = QObject::trUtf8("Не удалось открыть файл.");

		errorList.append(curError);

		return errorList;
	}

	QDomDocument domDocument;
	QString errorMsg;
	int errorLine, errorColumn;

	if (!domDocument.setContent(&xmlFile, &errorMsg, &errorLine, &errorColumn))
	{
		xmlFile.close();

		curError.mError = QObject::trUtf8("Не корректный xml: \"%1\" строка %2 столбец %3").arg(errorMsg).arg(errorLine).arg(errorColumn);

		errorList.append(curError);

		return errorList;
	}

	xmlFile.close();

	QDomElement rootElement = domDocument.documentElement();
	QDomElement sectionsElement = rootElement.firstChildElement("Sections");

	QMap <QString, QString> sections;

	if (sectionsElement.isElement())
	{
		sectionsElement = sectionsElement.firstChildElement();

		while (sectionsElement.isElement())
		{
			sections.insert(sectionsElement.tagName(), sectionsElement.attribute("name"));

			sectionsElement = sectionsElement.nextSiblingElement();
		}
	}

	SXParamData tempData;

	QDomElement recordsElement = rootElement.firstChildElement("Records");
	if (recordsElement.isElement())
	{
		recordsElement = recordsElement.firstChildElement();

		while (recordsElement.isElement())
		{
			tempData.mParamName = recordsElement.tagName();
			tempData.mParamDescr = recordsElement.attribute("name");
			tempData.mSectionName = recordsElement.attribute("sec");
			tempData.mType = recordsElement.attribute("type");

			tempData.mSectionDescr = sections.value(tempData.mSectionName);

			aParamsData.insert(tempData.mParamName, tempData);

			recordsElement = recordsElement.nextSiblingElement();
		}
	}

	return errorList;
}
