#include "CXSectionDialog.h"

#include "CXSettings.h"
#include "SXParamData.h"

//#include <QMessageBox>

CXSectionDialog::CXSectionDialog(const QList <SXParamData>& aReportData, QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);

	ui.mSectionList->clear();
	ui.mUserList->clear();

	QStringList tempSectList;
	QStringList tempUserList;
	QStringList ignoredSectionsList = CXSettings::inst()->value(E_IgnoredSections).toStringList();

	QListWidgetItem* newItem = NULL;

	//добавляем в список поле все
  newItem = new QListWidgetItem(trUtf8("Все"));
  newItem->setData(Qt::UserRole + 1, trUtf8("Все"));
  newItem->setCheckState(Qt::Checked);
  ui.mUserList->addItem(newItem);

	for (int i = 0; i < aReportData.count(); ++i)
	{
		const SXParamData& curData = aReportData.at(i);
		if (!tempSectList.contains(curData.mSectionName)){// continue;

      newItem = new QListWidgetItem(curData.mSectionDescr);
      newItem->setData(Qt::UserRole + 1, curData.mSectionName);
      if (ignoredSectionsList.contains(curData.mSectionName)) newItem->setCheckState(Qt::Unchecked);
      else newItem->setCheckState(Qt::Checked);

      ui.mSectionList->addItem(newItem);

      tempSectList.append(curData.mSectionName);
		};
		if(!tempUserList.contains(curData.userName)){
		  newItem = new QListWidgetItem(curData.userName);
      newItem->setData(Qt::UserRole + 1, curData.userName);
      newItem->setCheckState(Qt::Unchecked);
      ui.mUserList->addItem(newItem);
      tempUserList.append(curData.userName);
		}
	}
}

CXSectionDialog::~CXSectionDialog()
{

}

void CXSectionDialog::accept()
{
	QStringList ignoredSectionsList = CXSettings::inst()->value(E_IgnoredSections).toStringList();

	QString tempText;
	QListWidgetItem* curItem = NULL;
	for (int i = 0; i < ui.mSectionList->count(); ++i)
	{
		curItem = ui.mSectionList->item(i);

		tempText = curItem->data(Qt::UserRole + 1).toString();

		if (curItem->checkState() == Qt::Unchecked && !ignoredSectionsList.contains(tempText))
		{
			ignoredSectionsList.append(tempText);
		}

		if (curItem->checkState() == Qt::Checked && ignoredSectionsList.contains(tempText))
		{
			ignoredSectionsList.removeAt(ignoredSectionsList.indexOf((QRegExp)tempText));
		}
	}

	//выбираем пользователя
	CXSettings::inst()->setValue(E_UserName, trUtf8("Все"));
	for(int i = 0; i < ui.mUserList->count(); ++i){
	  curItem = ui.mUserList->item(i);
	  if(curItem->checkState() == Qt::Checked){
	    CXSettings::inst()->setValue(E_UserName, curItem->text());
	    break;
	  };
	};

	CXSettings::inst()->setValue(E_IgnoredSections, ignoredSectionsList);
	QDialog::accept();
}
