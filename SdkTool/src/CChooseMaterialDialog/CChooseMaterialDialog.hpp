#pragma once
#include <QDialog>
#include <QVector>
#include <QComboBox>
#include "../CObjSubmeshParser/CObjSubmeshParser.hpp"

class CChooseMaterialDialog : public QDialog {
	Q_OBJECT
public:
	explicit CChooseMaterialDialog(const QString& strObjPath,
		const QString& strTargetDirectory, const QString& strProjectDirectory,
		QWidget* pParent = nullptr);

	QString ObjPath() const { return m_strObjPath; }
	QString OutPath() const { return m_strOutPath; }
	QString MappingFilePath() const { return m_strMappingFile; }

private slots:
	void OnNext();
	void OnBack();
	void OnBrowseNew();
	void OnApply();

private:
	void SetupUi();
	void Refresh();
	void CollectFromCombo();
	void BuildMappingFile();
	
	QString m_strObjPath;
	QString m_strTargetDir;
	QString m_strProjectDir;
	QString m_strOutPath;
	QString m_strMappingFile;

	QVector<ObjSubmesh_t> m_Meshes;
	int m_nCurrent = 0;

	QLabel* m_pPreviewLabel = nullptr;
	QLabel* m_pTitleLabel = nullptr;
	QComboBox* m_pMaterialCombo = nullptr;
	QPushButton* m_pBackBtn = nullptr;
	QPushButton* m_pNextBtn = nullptr;
	QPushButton* m_pApplyBtn = nullptr;
	QPushButton* m_pNewMatBtn = nullptr;

	QVector<QString> m_vecChosen;
};