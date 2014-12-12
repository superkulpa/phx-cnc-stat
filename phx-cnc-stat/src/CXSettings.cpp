#include "CXSettings.h"

#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDomDocument>
#include <QStringList>

CXSettings* CXSettings::mSettings = NULL;

CXSettings* CXSettings::inst()
{
	if (mSettings == NULL) mSettings = new CXSettings;

	return mSettings;
}

CXSettings::CXSettings()
{
	mTypeDescription.insert(E_FTPHost,				"FTP Host");
	mTypeDescription.insert(E_TelnetUser,			"FTP User");
	mTypeDescription.insert(E_TelnetPassword,	"FTP Password");
	mTypeDescription.insert(E_StartDate,			"Start Date");
	mTypeDescription.insert(E_EndDate,				"End Date");
	mTypeDescription.insert(E_HeaderReport,			"Header Report");
	mTypeDescription.insert(E_FooterReport,			"Footer Report");
	mTypeDescription.insert(E_SectionReport,		"Section Report");
	mTypeDescription.insert(E_SectionExtReport,    "Section Ext Report");
	mTypeDescription.insert(E_SectionExtTextReport,    "Section Ext Text Report");
	mTypeDescription.insert(E_ParamReport,			"Section Param Report");
	mTypeDescription.insert(E_ParamExtReport,      "Section Param Ext Report");
	mTypeDescription.insert(E_ParamExtTextReport,      "Section Param Ext Text Report");
	mTypeDescription.insert(E_LogPeriod,			"Days to keep files");
	mTypeDescription.insert(E_IgnoredSections,		"Ignored Sections");
	mTypeDescription.insert(E_UserName,    "User Name");

	QString tmp = QApplication::applicationDirPath();
	load(tmp + "/settings.xml");
}

CXSettings::~CXSettings()
{
  QString tmp = QApplication::applicationDirPath();
	save(tmp + "/settings.xml");
}

void CXSettings::load(const QString& aFileName)
{
	QFile xmlFile(aFileName);

	if (xmlFile.open(QIODevice::ReadOnly))
	{
		QDomDocument domDocument;

		domDocument.setContent(&xmlFile);
		QDomElement rootElement = domDocument.documentElement();

		if (rootElement.tagName() == "Settings")
		{
			QDomElement paramElement = rootElement.firstChildElement("Param");
			while (paramElement.isElement())
			{
				mSettingsValue.insert(paramElement.attribute("type").toInt(), paramElement.attribute("value"));

				paramElement = paramElement.nextSiblingElement("Param");
			}
		}
	}
}

void CXSettings::save(const QString& aFileName)
{
	QFile xmlFile(aFileName);
	if (xmlFile.open(QIODevice::WriteOnly))
	{
		QTextStream textStream(&xmlFile);

		textStream << "<?xml version=\"1\"?>\n";
		textStream << "<Settings>\n";

		QMap <int, QVariant>::iterator iter;
		for (iter = mSettingsValue.begin(); iter != mSettingsValue.end(); ++iter)
		{
			textStream << QString("	<Param desrc=\"%1\" type=\"%2\" value=\"%3\"/>\n").arg(mTypeDescription.value(iter.key())).arg(iter.key()).arg(iter.value().toString().replace("&", "&amp;").replace('"', "&quot;"));
		}

		textStream << "</Settings>";

		xmlFile.close();
	}
}

void CXSettings::setValue(eSettingType aType, const QVariant& aValue)
{
	switch (aValue.type())
	{
		case QVariant::StringList:
		{
			mSettingsValue.insert(aType, aValue.toStringList().join("|||"));
			break;
		}
		default:
		{
			mSettingsValue.insert(aType, aValue);
			break;
		}
	}
}

QVariant CXSettings::value(eSettingType aType)
{
	switch (aType)
	{
		case E_IgnoredSections: return mSettingsValue.value(aType).toString().split("|||");

		default: return mSettingsValue.value(aType);
	}
}
