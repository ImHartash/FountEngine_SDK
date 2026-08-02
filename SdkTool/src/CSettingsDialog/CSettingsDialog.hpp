#pragma once
#include <QDialog>
#include <QLineEdit>

class CSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit CSettingsDialog(QWidget* pParent = nullptr);

private slots:
    void OnBrowseConvertersDir();
	void OnBrowseProjectDir();
    void OnBrowsePackOutputDir();
	void OnSaveClicked();

private:
    void SetupUi();
    void LoadCurrentValues();

    QLineEdit* m_pConvertersDirEdit;
	QLineEdit* m_pProjectDirEdit;
	QLineEdit* m_pPackNameEdit;
    QLineEdit* m_pPackOutputDirEdit;
};