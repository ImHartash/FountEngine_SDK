#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPlainTextEdit>
#include <QQueue>
#include <QMap>
#include <QDateTime>
#include "CAssetTreeView/CAssetTreeView.hpp"
#include "CAssetFileSystemModel/CAssetFileSystemModel.hpp"
#include "CFileIconProvider/CFileIconProvider.hpp"
#include "CConverterRunner/CConverterRunner.hpp"

struct PendingConversion_t {
	QString strExePath;
	QStringList listArgs;
	QString strDisplayName;
	QString strSourcePath;
	QString strOutputPath;
};

struct ConversionRecord_t {
	QString strSourceName;
	QDateTime When;
};

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(const QString& strProjectPath, QWidget* pParent = nullptr);

private slots:
	void OnFilesDropped(const QString& strTargetDir, const QStringList& listPaths);
	void OnSelectionChanged();
	void OnPackClicked();
	void OnLogLine(const QString& strLine);
	void OnConversionFinished(int nExitCode);
	void OnItemDoubleClicked(const QModelIndex& Index);

private:
	void SetupUi();
	void ProcessNextInQueue();
	void EnqueueConversion(const QString& strSourcePath, const QString& strTargetDir);
	void ReloadProject();
	void UpdateFileInfoPanel(const QString& strPath);
	void CheckFirstRun();
	QPixmap DecodeFntTex(const QString& strPath);
	QPixmap RenderWireframe(const QString& strPath);

	QString m_strProjectPath;

	CAssetFileSystemModel* m_pFileSystemModel;
	CFileIconProvider* m_pIconProvider;
	CAssetTreeView* m_pTreeView;
	QWidget* m_pTreeContainer;
	QLabel* m_pProjectNameLabel;

	QLabel* m_pPreviewIconLabel;
	QLabel* m_pPreviewNameLabel;
	QLabel* m_pPreviewMetaLabel;

	QLabel* m_pStatusIconLabel;
	QLabel* m_pStatusTextLabel;
	QPlainTextEdit* m_pLogConsole;

	CConverterRunner* m_pRunner;
	QQueue<PendingConversion_t> m_QueuePending;
	QMap<QString, ConversionRecord_t> m_ConversionHistory;
};