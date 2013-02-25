#include "CXSettingsDialog.h"

#include "CXSettings.h"

CXSettingsDialog::CXSettingsDialog(QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);
	
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChange(int)));
}

CXSettingsDialog::~CXSettingsDialog()
{

}

void CXSettingsDialog::showEvent(QShowEvent*)
{
	CXSettings* settings = CXSettings::inst();

	ui.mFTPHostEdit->setText(settings->value(E_FTPHost).toString());
	ui.mTelnetLoginEdit->setText(settings->value(E_TelnetUser).toString());
	ui.mTelnetPasswordEdit->setText(settings->value(E_TelnetPassword).toString());

	ui.mHeaderReportEdit->setText(settings->value(E_HeaderReport).toString());
	ui.mFooterReportEdit->setText(settings->value(E_FooterReport).toString());
	ui.mSectionReportEdit->setText(settings->value(E_SectionReport).toString());
	ui.mSectionExtReportEdit->setText(settings->value(E_SectionExtReport).toString());
	ui.mParamReportEdit->setText(settings->value(E_ParamReport).toString());
	ui.mParamExtReportEdit->setText(settings->value(E_ParamExtReport).toString());

	ui.mLogPeriodEdit->setValue(settings->value(E_LogPeriod).toInt());
}

void CXSettingsDialog::accept()
{
	CXSettings* settings = CXSettings::inst();

	settings->setValue(E_FTPHost,		      ui.mFTPHostEdit->text());
	settings->setValue(E_TelnetUser,		  ui.mTelnetLoginEdit->text());
	settings->setValue(E_TelnetPassword,	ui.mTelnetPasswordEdit->text());

	settings->setValue(E_HeaderReport,			ui.mHeaderReportEdit->text());
	settings->setValue(E_FooterReport,			ui.mFooterReportEdit->text());
	settings->setValue(E_SectionReport,			ui.mSectionReportEdit->text());
	settings->setValue(E_ParamReport,			ui.mParamReportEdit->text());

	settings->setValue(E_LogPeriod,				ui.mLogPeriodEdit->value());

	QDialog::accept();
}

void CXSettingsDialog::onCurrentTabChange(int aIndex)
{
	ui.listWidget->setCurrentRow(aIndex);
}
