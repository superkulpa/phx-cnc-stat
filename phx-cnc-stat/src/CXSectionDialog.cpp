#include "CXSectionDialog.h"

#include "CXSettings.h"
#include "SXParamData.h"

CXSectionDialog::CXSectionDialog(const QList <SXParamData>& aReportData, QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);

	ui.mSectionList->clear();

	QStringList tempList;
	QStringList ignoredSectionsList = CXSettings::inst()->value(E_IgnoredSections).toStringList();

	QListWidgetItem* newItem = NULL;
	for (int i = 0; i < aReportData.count(); ++i)
	{
		const SXParamData& curData = aReportData.at(i);

		if (tempList.contains(curData.mSectionName)) continue;

		newItem = new QListWidgetItem(curData.mSectionDescr);
		newItem->setData(Qt::UserRole + 1, curData.mSectionName);
		if (ignoredSectionsList.contains(curData.mSectionName)) newItem->setCheckState(Qt::Unchecked);
		else newItem->setCheckState(Qt::Checked);

		ui.mSectionList->addItem(newItem);

		tempList.append(curData.mSectionName);
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
			ignoredSectionsList.removeAt(ignoredSectionsList.indexOf(tempText));
		}
	}

	CXSettings::inst()->setValue(E_IgnoredSections, ignoredSectionsList);

	QDialog::accept();
}
