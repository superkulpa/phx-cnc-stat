#ifndef CXREPORTMANAGER_H
#define CXREPORTMANAGER_H

#include <QString>
#include <QDomDocument>
#include <QDateTime>
#include <QStringList>

//! Макрос относительного пути к папке для накапливания данных.
#define LOG_PATH "logs/CNC/logs"
//! Относительный путь копирования архивов
#define ARCHIVE_PATH "logs"

//! Структура с данными по ошибкам генерации отчета.
struct SXReportError
{
	QString mFileName;	//!< Имя файла.
	QString mError;		//!< Текст ошибки.
};

struct SXParamData;

//! Класс ядра по обработке и генерации отчета.
class Engine
{
public:
	/*!
		Статическая функци генерации отчета.

		\param aFileList - список xml-файлов для анализа и генерации отчета.
		\param aStartDate - Начальная дата интевала обрабатываемых данных.
		\param aEndDate - Конечная дата интевала обрабатываемых данных.

		\return Список ошибок при обработке данных.

		\note Отчет генерируется путем накапливания данных для параметров (суммирование их значений). В дальнейшем накопленные значения генерируются в отчет в зависимости от типа параметра.
	*/
	static QList <SXReportError> generateReport(const QStringList& aFileList, const QDateTime& aStartDate, const QDateTime& aEndDate);

	/*!
		Функция получения текста отчета.
		\return Текст отчета.
	*/
	static QString getReportText();
	/*!
		Функция сохранения сгенерированного отчета.
	*/
	static void saveReport();
	/*!
		Функция удаления лог-файлов, старше указанной в настройках даты.
		\sa CXSettings
	*/
	static void removeOldDirs();

	/*!
		Статическая функция заполнения списка описания параметра и его секции
	*/
	static QList <SXReportError> fillParamsData(QMap <QString, SXParamData>& aParamsData);

private:
	static QString mReportText;
};

#endif // CXREPORTMANAGER_H
