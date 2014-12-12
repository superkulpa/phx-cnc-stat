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
		return (mSectionName == aData.mSectionName
		     && mParamName == aData.mParamName
		     && userName == aData.userName);
	}

	/*!
		Получение значения параметра секции

		\return Значение параметра секции в зависимости от типа.
				Реализованные типы:
				- time - тип параметра, значением которого является время в секундах.
				- travel - тип параметра, значением которого является длина в 0,1 миллиметра.
				- другой тип является целочисленным и просто происходит его суммирование.
	*/
	QString getValue(bool _txt = 0) const
	{
		if( (mType == "time")
	    ||(mType == "info"))
		{
			if (_txt) return QObject::trUtf8("%1").arg(mValue);
			int hour = 0;
			int min = 0;
			int sec = 0;

			hour = mValue / 3600;
			min = mValue / 60 - hour * 60;
			sec = mValue - min * 60 - hour * 3600;

			if((hour <= 0) && (min <= 0)) return QObject::trUtf8("%1 сек.").arg(sec);
			else if(hour <= 0) return QObject::trUtf8("%1 мин. %2 сек.").arg(min).arg(sec);
			return QObject::trUtf8("%1 ч. %2 мин. %3 сек.").arg(hour).arg(min).arg(sec);
		}

		if (mType == "travel")
		{
			if (_txt) return QObject::trUtf8("%1").arg(mValue);
			int value = mValue;// / 10.0;

			if (value < 1000) return QObject::trUtf8("%1 мм").arg(value);

			int meters = value / 1000;
			value %= 1000;

			return QObject::trUtf8("%1 м. %2 мм").arg(meters).arg(value);
		}

		if (mType == "name")
    {
      return QObject::trUtf8(sValue.toStdString().c_str());
    }

    return QString::number(mValue);
	}

	QString mSectionName;	//!< Имя секции.
	QString mSectionDescr;	//!< Описании секции.

	QString mParamName;		//!< Имя параметра.
	QString mParamDescr;	//!< Описание параметра.

	int mValue;				//!< Значение параметра.
	QString sValue;   //!< Значение параметра строки.
	QString mType;			//!< Тип параметра.

	QString userName; //!< имя резчика.

};

#endif // SXPARAMDATA_H
