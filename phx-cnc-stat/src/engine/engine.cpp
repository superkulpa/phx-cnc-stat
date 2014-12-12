#include "engine.h"

#include <QFile>
#include <QApplication>
#include <QTextStream>
#include <QTextCodec>
#include <QFileDialog>
#include <QMessageBox>

#include "../CXSettings.h"
#include "../CXSectionDialog.h"
#include "../SXParamData.h"

QString Engine::mReportText;
QString Engine::mText;

QMap <QString, QString>Engine::sections;

void CombineReport(QList <SXParamData>& _reportData){
  QList<QString> sectionName;
  QList<QString> paramName;
  QMap<QString,int> indxMap;
  for(int i = 0; i < _reportData.size(); i++){
    SXParamData curData = _reportData.at(i);
    //объединяем только параметры, связанные с работой машины
    if(curData.mSectionName == "Info") continue;
    if( (sectionName.contains(curData.mSectionName))
    &&  (paramName.contains(curData.mParamName))){
      //совпало суммируем
      _reportData[indxMap[curData.mParamName]].mValue += curData.mValue;
      //удаляем за ненадобностью
      _reportData.removeAt(i); i--;
    }else{
      sectionName.append(curData.mSectionName);
      paramName.append(curData.mParamName);
      //сохраняем позицию
      indxMap[curData.mParamName] = i;
    };
  };
};


QList <SXReportError> Engine::generateReport(const QStringList& aFileList, const QDateTime& aStartDate, const QDateTime& aEndDate)
{
	QMap <QString, SXParamData> paramData;
	QList <SXReportError> errorList = fillParamsData(paramData);


  QString tmp =   QApplication::applicationDirPath();

	if (aStartDate >= aEndDate)
	{
		QMessageBox::critical(NULL, QObject::trUtf8("Ошибка"), QObject::trUtf8("Не верный интервал времени.\nДата До должна быть позже чем дата От."));
		return errorList;
	}

	SXReportError curError;

	CXSettings* settings = CXSettings::inst();

	QFile xmlFile;
	QDomDocument domDocument;
	QDateTime curDate;
	QDomElement rootElement, sectionElement, paramElement;

	QFile headerHTML(tmp + "/" + settings->value(E_HeaderReport).toString());

	mReportText = QObject::trUtf8("<html><body>С {Start Date} по {End Date} пользователь: {User Name} <pre>");
	mText = QObject::trUtf8("С {Start Date} по {End Date} пользователь: {User Name}\n");

	if (headerHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&headerHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		mReportText = textStream.readAll();
	}

	mReportText = mReportText.replace("{Start Date}",	aStartDate.toString("dd.MM.yyyy hh:mm:ss"));
	mReportText = mReportText.replace("{End Date}",		aEndDate.toString("dd.MM.yyyy hh:mm:ss"));
	mText = mText.replace("{Start Date}",	aStartDate.toString("dd.MM.yyyy hh:mm:ss"));
	mText = mText.replace("{End Date}",	aStartDate.toString("dd.MM.yyyy hh:mm:ss"));
/**/
	QFile itemHTML(tmp + "/" + settings->value(E_SectionReport).toString());

	QString sectionTemplate = QObject::trUtf8("{Section Descr}\n{Params}");

	QString sectionTextTemplate = QObject::trUtf8("{Section Descr}\n{Params}");

	if (itemHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&itemHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		sectionTemplate = textStream.readAll();
	}

	itemHTML.close();

	itemHTML.setFileName(tmp + "/" + settings->value(E_SectionExtReport).toString());

  QString sectionExtTemplate = QObject::trUtf8("{Section Descr}\n{Params}");

  if (itemHTML.open(QIODevice::ReadOnly))
  {
    QTextStream textStream(&itemHTML);
    textStream.setCodec(QTextCodec::codecForName("UTF-8"));

    sectionExtTemplate = textStream.readAll();
  }

  itemHTML.close();

  itemHTML.setFileName(tmp + "/" + settings->value(E_SectionExtTextReport).toString());

  QString sectionExtTextTemplate = QObject::trUtf8("{Section Descr}\n{Params}");

	if (itemHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&itemHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		sectionExtTextTemplate = textStream.readAll();
	}

	itemHTML.close();


	//обычные параметры
	itemHTML.setFileName(tmp + "/" + settings->value(E_ParamReport).toString());

	QString paramTemplate = QObject::trUtf8("	{Param User} {Param Descr}	{Param Value}\n");
	QString paramTextTemplate = QObject::trUtf8("	{Param User} {Param Descr}	{Param Value}\n");

	if (itemHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&itemHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		paramTemplate = textStream.readAll();
	}

	itemHTML.close();

  //информация по УП
  itemHTML.setFileName(tmp + "/" + settings->value(E_ParamExtReport).toString());

  QString paramExtTemplate = QObject::trUtf8(" {Param User} {Param Descr}  {Param Value}\n");

  if (itemHTML.open(QIODevice::ReadOnly))
  {
    QTextStream textStream(&itemHTML);
    textStream.setCodec(QTextCodec::codecForName("UTF-8"));

    paramExtTemplate = textStream.readAll();
  }
  itemHTML.close();

  itemHTML.setFileName(tmp + "/" + settings->value(E_ParamExtTextReport).toString());
  QString paramExtTextTemplate = QObject::trUtf8(" {Param User} {Param Descr}  {Param Value}\n");
  if (itemHTML.open(QIODevice::ReadOnly))
	{
		QTextStream textStream(&itemHTML);
		textStream.setCodec(QTextCodec::codecForName("UTF-8"));

		paramExtTextTemplate = textStream.readAll();
	}
  itemHTML.close();

/**/
	QList <SXParamData> reportData;
	//лист расширенных параметров
	QList <SXParamData> reportDataExt;

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
		SXParamData cpData; cpData.mSectionName = "Info"; cpData.mParamName = "CP_Time";
    cpData.mValue = -1;

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
				curData.mValue = paramElement.attribute("value").toFloat();

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
        //считываем имя пользователя
        curData.userName = rootElement.attribute("user_name");

				//проверяем не пора ли сбросить статистику
				if(curData.mParamName == "T_Load"){
				  //если нада скидываем
				  if(cpData.mValue != -1)
				    reportData.append(cpData);
				  cpData.mValue = 0;
				  cpData.userName = curData.userName;
				  curData.mValue = paramElement.attribute("time").toFloat() / ticToSec;
				  //вычисляем абсолютное время
				  curData.sValue = curDate.addSecs(curData.mValue).toString("dd.MM.yyyy hh:mm");
				}

				if(curData.mParamName == "CP_Time"){
          if(cpData.mValue == 0){
            //записываем названия и продолжаем
            cpData = curData;
          }else{
            cpData.mValue += curData.mValue;
          }
          paramElement = paramElement.nextSiblingElement();
          continue;
        };

        if(curData.mParamName == "CP_Name")
          curData.sValue = paramElement.attribute("value");

				//если уже есть такой параметр - суммировать значение.
				if (reportData.contains(curData)
				 && curData.mSectionName != "Info")
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
		//добавляем последнее время отработки если нада
		if(cpData.mValue != -1)
		  reportData.append(cpData);
	}


/**/
	if (reportData.isEmpty()){
		mReportText.append(QObject::trUtf8("Нет накопленных данных"));
		mText.append(QObject::trUtf8("Нет накопленных данных"));
	}	else
	{
		CXSectionDialog sectionDialog(reportData);
		sectionDialog.exec();

		QStringList ignoredSectionsList = CXSettings::inst()->value(E_IgnoredSections).toStringList();
		QString userName = CXSettings::inst()->value(E_UserName).toString();

		QString sectionName;
		QString sectionText, paramText;
		QString sectionName_t;
		QString sectionText_t, paramText_t;

		QString paramExtText = paramExtTemplate;
		QString paramExtText_t = paramExtTextTemplate;

		if(userName == QObject::trUtf8("Все")){
      //собрать одинаковые поля по разным пользователям
		  CombineReport(reportData);
    }

		//Заполнение данных в отчет из списка mReportData.
		int nSect = 0;
		while (!reportData.isEmpty())
		{
			//const SXParamData& curSection = reportData.first();
			//sectionName = curSection.mSectionName;
			sectionName = sections.keys().at(nSect++);
			//вставляем имя пользователя
      mReportText = mReportText.replace("{User Name}", userName);
      mText = mText.replace("{User Name}", userName);

			if (ignoredSectionsList.contains(sectionName))
			{
				//reportData.removeFirst();

				continue;
			}
			paramText.clear();
			paramText_t.clear();
			//выбираем макет построения
			if(sectionName == "Info"){
			  sectionText = sectionExtTemplate;
			  sectionText_t = sectionExtTextTemplate;
			  sectionText = sectionText.replace("{CP Name}", QObject::trUtf8("Имя УП"))
			                           .replace("{T Load}", QObject::trUtf8("Время загрузки"))
			                           .replace("{B Count}", QObject::trUtf8("Пробивки"))
			                           .replace("{L Burn}", QObject::trUtf8("Длина реза"))
			                           .replace("{CP Time}", QObject::trUtf8("Время отработки"));
			  sectionText_t = sectionText_t.replace("{CP Name}", QObject::trUtf8("Имя УП"))
																 .replace("{T Load}", QObject::trUtf8("Время загрузки"))
																 .replace("{B Count}", QObject::trUtf8("Пробивки"))
																 .replace("{L Burn}", QObject::trUtf8("Длина реза"))
																 .replace("{CP Time}", QObject::trUtf8("Время отработки"));
			}else{
			  sectionText = sectionTemplate;
			  sectionText_t = "";//sectionTextTemplate;

			  sectionText = sectionText.replace("{Section}", sectionName/*curSection.mSectionName*/)
			               .replace("{Section Descr}", sections.value(sectionName));
			  sectionText_t = sectionText_t.replace("{Section}", sectionName/*curSection.mSectionName*/)
										 .replace("{Section Descr}", sections.value(sectionName));
			}

			for (int i = 0; i < reportData.count(); ++i)
			{
				const SXParamData& curData = reportData.at(i);
				if(ignoredSectionsList.contains(curData.mSectionName)){
				  reportData.removeAt(i);
          --i;
          continue;
				}
				if((userName != curData.userName) && (userName != QObject::trUtf8("Все"))) {
				  reportData.removeAt(i);
          --i;
				  continue;
        }
				//разбираем параметры: Info в одну таблицу, остальное в другую
        if((curData.mSectionName == "Info")
        && (sectionName == "Info")){
          QString name = "{"; name += curData.mParamName; name += "}";
          paramExtText.replace(name, curData.getValue());
          if((curData.mParamName != "CP_Time") || (curData.mParamName != "LBurn")){
          	paramExtText_t.replace(name, curData.getValue(1));
					}else
						paramExtText_t.replace(name, curData.getValue());

          if(curData.mParamName != "CP_Time"){
            reportData.removeAt(i);
            --i;
            continue;
          }
          //если значение времени отработки 0 то просто пропускаем
          if(curData.mValue == 0){
            reportData.removeAt(i);
            --i;
            paramExtText = paramExtTemplate;
            paramExtText_t = paramExtTextTemplate;
            continue;
          }
          //скидываем накопленную статистику
          reportData.removeAt(i);
          --i;
          paramText.append(paramExtText);
          paramText_t.append(paramExtText_t);
          paramExtText = paramExtTemplate;
          paramExtText_t = paramExtTextTemplate;
        }else if (curData.mSectionName == sectionName)
				{
				  paramText.append(QString(paramTemplate).replace("{Param Name}", curData.mParamName)
                                                 .replace("{Param User}", userName)
                                                 .replace("{Param Descr}", curData.mParamDescr)
                                                 .replace("{Param Value}", curData.getValue()));
				  paramText_t.append(QString(paramTextTemplate).replace("{Param Name}", curData.mParamName)
																											 .replace("{Param User}", userName)
																											 .replace("{Param Descr}", curData.mParamDescr)
																											 .replace("{Param Value}", curData.getValue()));
					reportData.removeAt(i);
					--i;
				}
			}
			if(paramText.size() > 1){
			  sectionText = sectionText.replace("{Params}", paramText);
			  sectionText_t = sectionText_t.replace("{Params}", paramText_t);
			  mReportText.append(sectionText);
			  if(sectionText_t.size() > 1)
			  	mText.append(sectionText_t);
			}
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
	QString fileName = QFileDialog::getSaveFileName(NULL, QObject::trUtf8("Сохранение отчета"), QString(), "HTML (*.html);;Text files (*.txt)");

	if (fileName.isEmpty()) return;

	QFile file(fileName);

	if (!file.open(QIODevice::WriteOnly)) return;

	QTextStream textStream(&file);
	textStream.setCodec(QTextCodec::codecForName("UTF-8"));
	textStream << mReportText;

	file.close();

	QFile fileTXT(fileName+".xls");

	if (!fileTXT.open(QIODevice::WriteOnly)) return;

	QTextStream textStreamTXT(&fileTXT);
	textStreamTXT.setCodec(QTextCodec::codecForName("UTF-8"));
	textStreamTXT << mText;

	fileTXT.close();
}

void Engine::removeOldDirs()
{
	int logDays = CXSettings::inst()->value(E_LogPeriod).toInt();

	//если журнал хранится ограниченное количество дней.
	if (logDays > 0)
	{
	  QString tmp = QApplication::applicationDirPath();
		QDir logsDir(tmp + "/" + LOG_PATH);

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

	QString tmp = QApplication::applicationDirPath();
	QString fileName = tmp + "/descrips.xml";

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
