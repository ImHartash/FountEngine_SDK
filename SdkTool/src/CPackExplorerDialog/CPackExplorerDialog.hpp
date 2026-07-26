#pragma once
#include <QDialog>
#include <QListWidget>
#include <QString>
#include <QMap>
#include <QByteArray>

struct PackEntry_t {
    QString strName;
    QByteArray Data;
};

class CPackExplorerDialog : public QDialog {
    Q_OBJECT

public:
    explicit CPackExplorerDialog(const QString& strPackPath, QWidget* pParent = nullptr);

private slots:
	void OnAddFileClicked();
	void OnRemoveClicked();
	void OnSaveClicked();

private:
	void SetupUi();
	bool LoadPackage();
	bool SavePackage();
	void RefreshList();

	QString m_strPackPath;
	QListWidget* m_pListWidget;
	QVector<PackEntry_t> m_vecEntries;
};