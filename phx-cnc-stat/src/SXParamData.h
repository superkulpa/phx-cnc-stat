#ifndef SXPARAMDATA_H
#define SXPARAMDATA_H

#include <QTime>

//! Структура для описания параметра в отчете.
struct SXParamData
{
	SXParamData()
	{
		mValue = 0;
	}

	//! Определенный оператор сравнения (для организации сортировки в списке).
	bool operator == (const SXParamData& aData)
	{
		return (mSectionName == aData.mSectionName && mParamName == aData.mParamName);
	}

	/*!
		Получение значения параметра секции

		\return Значение параметра секции в зависимости от типа.
				Реализованные типы:
				- time - тип параметра, значением которого является время в секундах.
				- travel - тип параметра, значением которого является длина в 0,1 миллиметра.
				- другой тип является целочисленным и просто происходит его суммирование.
	*/
	QString getValue() const
	{
		if (mType == "time")
		{
			int hour = 0;
			int min = 0;

			hour = mValue / 3600;
			min = mValue / 60 - hour * 60;

			if (hour <= 0) return QObject::trUtf8("%1 мин.").arg(min);

			return QObject::trUtf8("%1 ч. %2 мин.").arg(hour).arg(min);
		}

		if (mType == "travel")
		{
			int value = mValue / 10.0;

			if (value < 1000) return QObject::trUtf8("%1 мм").arg(value);

			int meters = value / 1000;
			value %= 1000;

			return QObject::trUtf8("%1 м. %2 мм").arg(meters).arg(value);
		}

		return QString::number(mValue);
	}

	QString mSectionName;	//!< Имя секции.
	QString mSectionDescr;	//!< Описании секции.

	QString mParamName;		//!< Имя параметра.
	QString mParamDescr;	//!< Описание параметра.

	int mValue;				//!< Значение параметра.
	QString mType;			//!< Тип параметра.
};

#endif // SXPARAMDATA_H
