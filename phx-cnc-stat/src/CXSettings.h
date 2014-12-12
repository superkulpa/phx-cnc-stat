#ifndef CXSETTINGS_H
#define CXSETTINGS_H

#include <QMap>
#include <QVariant>

//! Перечень доступных настроек.
enum eSettingType
{
	E_None = 0,

	E_FTPHost,				//!< Адрес сервера.

	E_TelnetUser,				//!< Логин для Telnet-соединения.
	E_TelnetPassword,		//!< Пароль для Telnet-соединения.

	E_StartDate,			//!< Дата для левой границы интервала отбора.
	E_EndDate,				//!< Дата для правой границы интервала отбора.

	E_HeaderReport,			//!< Путь к шаблону шапки отчета.
	E_FooterReport,			//!< Путь к шаблону подвала отчета.
	E_SectionReport,		//!< Путь к шаблону секции отчета.
	E_SectionExtReport, //!< Путь к шаблону секции информации по УП.
	E_SectionExtTextReport, //!< Путь к шаблону секции информации по УП(текстовой).
	E_ParamReport,			//!< Путь к шаблону поля секции отчета.
	E_ParamExtReport,   //!< Путь к шаблону поля информации по УП.
	E_ParamExtTextReport,   //!< Путь к шаблону поля информации по УП(текстовой).

	E_LogPeriod,			//!< Периол хранения журнала (дней).
	E_IgnoredSections,	//!< Список секций, которые не попадудт в отчет.
	E_UserName        //!< Имя пользователя, для которого генерируется отчет
};

/*!
	Singleton класс хранения, получения и сохранения настроек приложения.

	\note Для настроек E_CreateArchiveScript и E_ArchiverPath доступна текстовые переменная [APP_PATH], вместо которой будет автоматически подставлен путь до папки, в которой находится исполняемы файл программы.
		  Для E_ArchiverPath доступна также текстовая переменная [ARCHIVE_NAME], которая будет автоматически заменена на путь к файлу распаковываемого архива. Данный путь будет генерироваться программно.
		  Для шаблона E_HeaderReport доступны переменные {Start Date} и {End Date}, которые будут заменены на начальную и конечную даты интервала генерации отчета соответственно.
		  Для шаблона E_SectionReport доступны переменные {Section}, {Section Descr} и {Params}.
		  {Section} будет заменена на название секции (атрибут name тега Section в xml-файле), {Section Descr} - на описание (атрибут descr тега Section), {Params} - на текст описания всех параметров текущей секции.
		  Для шаблона E_ParamReport доступны переменные {Param Name}, {Param Descr} и {Param Value}.
		  {Param Name} будет заменена на название параметра секции (атрибут name тега Param в xml-файле), {Param Descr} - на описание (атрибут descr тега Param), {Param Value} - на значение параметра \sa SXParamData::getValue().

*/
class CXSettings
{
public:
	/*!
		Статическая функция полечения экземпляра класса в соответствии с паттерном Singleton.
	*/
	static CXSettings* inst();

private:
	CXSettings();

public:
	~CXSettings();

	/*!
		Функция загрузки настроек из xml-файла.

		\param aFileName - имя загружаемого xml-файла.
	*/
	void load(const QString& aFileName);
	/*!
		Функция сохранения настроек в xml-файл.

		\param aFileName - имя сохраняемого xml-файла.
	*/
	void save(const QString& aFileName);

	/*!
		Функция установки значения параметра настройки.

		\param aType - тип параметра настройки.
		\param aValue - значение параметра настройки.
	*/
	void setValue(eSettingType aType, const QVariant& aValue);
	/*!
		Функция получения значения параметра настройки.

		\param aType - тип параметра настройки.

		\return Значение параметра настройки.
	*/
	QVariant value(eSettingType aType);

private:
	static CXSettings* mSettings;

	QMap <int, QVariant> mSettingsValue;

	QMap <int, QString> mTypeDescription;
};

#endif // CXSETTINGS_H
