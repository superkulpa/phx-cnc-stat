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
	ui.mFTPPortEdit->setValue(settings->value(E_FTPPort).toInt());
	ui.mFTPLoginEdit->setText(settings->value(E_FTPUser).toString());
	ui.mFTPPasswordEdit->setText(settings->value(E_FTPPassword).toString());
	ui.mFTPDirEdit->setText(settings->value(E_FTPDir).toString());

	ui.mArchiverPathEdit->setText(settings->value(E_ArchiverPath).toString());
	ui.mCreateScriptEdit->setText(settings->value(E_CreateArchiveScript).toString());
	ui.mArchiveNameEdit->setText(settings->value(E_ArchiveName).toString());

	ui.mHeaderReportEdit->setText(settings->value(E_HeaderReport).toString());
	ui.mFooterReportEdit->setText(settings->value(E_FooterReport).toString());
	ui.mSectionReportEdit->setText(settings->value(E_SectionReport).toString());
	ui.mParamReportEdit->setText(settings->value(E_ParamReport).toString());

	ui.mLogPeriodEdit->setValue(settings->value(E_LogPeriod).toInt());
}

void CXSettingsDialog::accept()
{
	CXSettings* settings = CXSettings::inst();

	settings->setValue(E_FTPHost,		ui.mFTPHostEdit->text());
	settings->setValue(E_FTPPort,		ui.mFTPPortEdit->value());
	settings->setValue(E_FTPUser,		ui.mFTPLoginEdit->text());
	settings->setValue(E_FTPPassword,	ui.mFTPPasswordEdit->text());
	settings->setValue(E_FTPDir,		ui.mFTPDirEdit->text());

	settings->setValue(E_ArchiverPath,			ui.mArchiverPathEdit->text());
	settings->setValue(E_CreateArchiveScript,	ui.mCreateScriptEdit->text());
	settings->setValue(E_ArchiveName,			ui.mArchiveNameEdit->text());

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
