#ifndef CXSETTINGSDIALOG_H
#define CXSETTINGSDIALOG_H

#include "ui_settings.h"

#include "CXSettings.h"

//! Класс диалога настройки программы.
class CXSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	CXSettingsDialog(QWidget* parent = 0);
	~CXSettingsDialog();

protected:
	virtual void showEvent(QShowEvent*);

private slots:
	//! Переопределенный слот для применения настроек.
	virtual void accept();
	//! Слот изменения текущей вкладки настроек для синхронизации со списком.
	void onCurrentTabChange(int aIndex);

private:
	Ui::SettingsDialog ui;
};

#endif // CXSETTINGSDIALOG_H
