#include "CChooseMaterialDialog.hpp"
#include "../CDX11Renderer/CDX11Renderer.hpp"
#include "../CMaterialCreatorDialog/CMaterialCreatorDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QFile>
#include <QPixmap>
#include <QTextStream>
#include <QDateTime>

CChooseMaterialDialog::CChooseMaterialDialog(const QString& strObjPath,
	const QString& strTargetDirectory, const QString& strProjectDirectory,
	QWidget* pParent) : QDialog(pParent), m_strObjPath(strObjPath), m_strTargetDir(strTargetDirectory),
	m_strProjectDir(strProjectDirectory)
{
	if (!CObjSubmeshParser::Parse(m_strObjPath, m_Meshes) || m_Meshes.isEmpty()) {
		QMessageBox::critical(this, "Failed to parse file", "No submeshes (usemtl groups) found in .obj.");
		reject();
		return;
	}

	m_vecChosen.resize(m_Meshes.size());
	for (int i = 0; i < m_Meshes.size(); ++i)
		m_vecChosen[i].clear();

	SetupUi();
	Refresh();
}

void CChooseMaterialDialog::OnNext() {
	if (m_nCurrent < m_Meshes.size() - 1) { CollectFromCombo(); ++m_nCurrent; Refresh(); }
}

void CChooseMaterialDialog::OnBack() {
	if (m_nCurrent > 0) { CollectFromCombo(); --m_nCurrent; Refresh(); }
}

void CChooseMaterialDialog::OnBrowseNew() {
	CMaterialCreatorDialog Dialog(m_strTargetDir, this);
	if (Dialog.exec() == QDialog::Accepted)
		Refresh();
}

void CChooseMaterialDialog::OnApply() {
	CollectFromCombo();
	BuildMappingFile();

	m_strOutPath = m_strTargetDir + "/" +
		QFileInfo(m_strObjPath).completeBaseName() + ".fntmdl";
	accept();
}

void CChooseMaterialDialog::SetupUi() {
	setWindowTitle("Assign material");
	resize(380, 440);

	auto* pLayout = new QVBoxLayout(this);

	m_pTitleLabel = new QLabel();
	m_pTitleLabel->setAlignment(Qt::AlignCenter);
	pLayout->addWidget(m_pTitleLabel);

	m_pPreviewLabel = new QLabel();
	m_pPreviewLabel->setAlignment(Qt::AlignCenter);
	m_pPreviewLabel->setFixedSize(340, 340);
	m_pPreviewLabel->setStyleSheet("background:#1d1d22; border:1px solid #333; border-radius:6px;");
	pLayout->addWidget(m_pPreviewLabel, 1);

	auto* pComboRow = new QHBoxLayout();
	pComboRow->addWidget(new QLabel("Material:"));
	m_pMaterialCombo = new QComboBox();
	pComboRow->addWidget(m_pMaterialCombo, 1);
	m_pNewMatBtn = new QPushButton("New...");
	pComboRow->addWidget(m_pNewMatBtn);
	pLayout->addLayout(pComboRow);

	auto* pBtnRow = new QHBoxLayout();
	m_pBackBtn = new QPushButton("< Back");
	m_pNextBtn = new QPushButton("Next >");
	m_pApplyBtn = new QPushButton("Apply & Convert");
	pBtnRow->addWidget(m_pBackBtn);
	pLayout->addLayout(pBtnRow);
	pLayout->addWidget(m_pNextBtn);
	pLayout->addWidget(m_pApplyBtn);

	connect(m_pNextBtn, &QPushButton::clicked, this, &CChooseMaterialDialog::OnNext);
	connect(m_pBackBtn, &QPushButton::clicked, this, &CChooseMaterialDialog::OnBack);
	connect(m_pNewMatBtn, &QPushButton::clicked, this, &CChooseMaterialDialog::OnBrowseNew);
	connect(m_pApplyBtn, &QPushButton::clicked, this, &CChooseMaterialDialog::OnApply);
}

void CChooseMaterialDialog::Refresh() {
	int nTotal = m_Meshes.size();
	CollectFromCombo();

	m_pTitleLabel->setText(QString("Mesh %1 of %2 — choose a material\n(%3)")
		.arg(m_nCurrent + 1).arg(nTotal)
		.arg(m_Meshes[m_nCurrent].strMaterial.isEmpty()
			? "(none)" : m_Meshes[m_nCurrent].strMaterial));

	// Render the image
	CDX11Renderer R;
	std::vector<PreviewVertex_t> vecVertices(m_Meshes[m_nCurrent].vecVertices.begin(), m_Meshes[m_nCurrent].vecVertices.end());
	std::vector<uint32_t> vecIndices(m_Meshes[m_nCurrent].vecIndices.begin(), m_Meshes[m_nCurrent].vecIndices.end());
	QImage Img = R.RenderImage(vecVertices, vecIndices, 340, 340);
	m_pPreviewLabel->setPixmap(QPixmap::fromImage(Img));

	// =====
	//m_pMaterialCombo->clear();
	//m_pMaterialCombo->addItem("(none)", QString());
	//QDir Dir(m_strTargetDir);
	//for (const QString& p : Dir.entryList({ "*.fntmat" }, QDir::Files))
	//	m_pMaterialCombo->addItem("materials/" + p, "materials/" + p);

	//if (!m_vecChosen[m_nCurrent].isEmpty())
	//	m_pMaterialCombo->setCurrentText(m_vecChosen[m_nCurrent]);

	//m_pBackBtn->setEnabled(m_nCurrent > 0);
	//bool bLast = (m_nCurrent >= nTotal - 1);
	//m_pNextBtn->setVisible(!bLast);
	//m_pApplyBtn->setVisible(bLast);

	m_pMaterialCombo->clear();
	m_pMaterialCombo->addItem("(none)", QString());
	QDir ProjectDir(m_strProjectDir);

	QDirIterator DirIter(m_strProjectDir, { "*.fntmat" }, QDir::Files, QDirIterator::Subdirectories);
	while (DirIter.hasNext()) {
		DirIter.next();
		QString strRelativePath = ProjectDir.relativeFilePath(DirIter.filePath());
		m_pMaterialCombo->addItem(strRelativePath, strRelativePath);
	}

	if (!m_vecChosen[m_nCurrent].isEmpty()) {
		int nIndex = m_pMaterialCombo->findData(m_vecChosen[m_nCurrent]);
		if (nIndex >= 0) m_pMaterialCombo->setCurrentIndex(nIndex);
	}

	m_pBackBtn->setEnabled(m_nCurrent > 0);
	bool bLast = (m_nCurrent >= nTotal - 1);
	m_pNextBtn->setVisible(!bLast);
	m_pApplyBtn->setVisible(bLast);
}

void CChooseMaterialDialog::CollectFromCombo() {
	if (m_nCurrent >= 0 && m_nCurrent < m_vecChosen.size()) {
		m_vecChosen[m_nCurrent] = m_pMaterialCombo->currentData().toString();
	}
}

void CChooseMaterialDialog::BuildMappingFile() {
	m_strMappingFile = QDir::temp().filePath(
		"fntmdl_map_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".txt");

	QFile File(m_strMappingFile);
	if (!File.open(QIODevice::WriteOnly)) return;

	for (int i = 0; i < m_Meshes.size(); ++i) {
		QString strName = m_Meshes[i].strMaterial;
		QString strPath = m_vecChosen[i];

		if (strName.isEmpty()) strName = "default";
		if (strPath.isEmpty()) continue;

		File.write(qPrintable(strName + " " + strPath + "\n"));
	}

	File.close();
}