#ifndef CXSECTIONDIALOG_H
#define CXSECTIONDIALOG_H

#include "ui_CXSectionDialog.h"

struct SXParamData;

class CXSectionDialog : public QDialog
{
	Q_OBJECT

public:
	CXSectionDialog(const QList <SXParamData>& aReportData, QWidget* parent = 0);
	~CXSectionDialog();

public slots:
	virtual void accept();

private:
	Ui::SectionDialog ui;
};

#endif // CXSECTIONDIALOG_H
