#include "CMaterialCreatorDialog.hpp"
#include "../CSettingsManager/CSettingsManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QFileInfo>

CMaterialCreatorDialog::CMaterialCreatorDialog(const QString& strTargetDir, QWidget* pParent)
    : QDialog(pParent), m_strTargetDir(strTargetDir) {
    
    setWindowTitle("Create New Material");
    resize(380, 480);
    SetupUi();
}

QDoubleSpinBox* CMaterialCreatorDialog::MakeColorSpinBox() {
	auto* pSpinBox = new QDoubleSpinBox();
	pSpinBox->setRange(0.0, 1.0);
	pSpinBox->setSingleStep(0.05);
	pSpinBox->setValue(1.0);
	return pSpinBox;
}

void CMaterialCreatorDialog::SetupUi() {
	auto* pMainLayout = new QVBoxLayout(this);
	auto* pForm = new QFormLayout();

	m_pNameEdit = new QLineEdit("new_material");
	pForm->addRow("Material name:", m_pNameEdit);

	auto* pTextureRow = new QHBoxLayout();
	m_pTextureEdit = new QLineEdit();
	auto* pBrowseButton = new QPushButton("Browse...");
	connect(pBrowseButton, &QPushButton::clicked, this, &CMaterialCreatorDialog::OnBrowseTexture);
	pTextureRow->addWidget(m_pTextureEdit);
	pTextureRow->addWidget(pBrowseButton);
	pForm->addRow("Diffuse texture:", pTextureRow);

	m_pAmbientR = MakeColorSpinBox(); m_pAmbientG = MakeColorSpinBox(); m_pAmbientB = MakeColorSpinBox();
	auto* pAmbientRow = new QHBoxLayout();
	pAmbientRow->addWidget(m_pAmbientR); pAmbientRow->addWidget(m_pAmbientG); pAmbientRow->addWidget(m_pAmbientB);
	pForm->addRow("Ambient (R,G,B):", pAmbientRow);

	m_pDiffuseR = MakeColorSpinBox(); m_pDiffuseG = MakeColorSpinBox(); m_pDiffuseB = MakeColorSpinBox();
	auto* pDiffuseRow = new QHBoxLayout();
	pDiffuseRow->addWidget(m_pDiffuseR); pDiffuseRow->addWidget(m_pDiffuseG); pDiffuseRow->addWidget(m_pDiffuseB);
	pForm->addRow("Diffuse (R,G,B):", pDiffuseRow);

	m_pSpecularR = MakeColorSpinBox(); m_pSpecularG = MakeColorSpinBox(); m_pSpecularB = MakeColorSpinBox();
	auto* pSpecularRow = new QHBoxLayout();
	pSpecularRow->addWidget(m_pSpecularR); pSpecularRow->addWidget(m_pSpecularG); pSpecularRow->addWidget(m_pSpecularB);
	pForm->addRow("Specular (R,G,B):", pSpecularRow);

	m_pShininess = new QDoubleSpinBox();
	m_pShininess->setRange(1.0, 256.0);
	m_pShininess->setValue(32.0);
	pForm->addRow("Shininess:", m_pShininess);

	m_pOpacity = new QDoubleSpinBox();
	m_pOpacity->setRange(0.0, 1.0);
	m_pOpacity->setSingleStep(0.05);
	m_pOpacity->setValue(1.0);
	pForm->addRow("Opacity:", m_pOpacity);

	m_pBlendModeCombo = new QComboBox();
	m_pBlendModeCombo->addItems({"Opaque", "AlphaBlend", "Additive"});
	pForm->addRow("Blend mode:", m_pBlendModeCombo);

	m_pCullModeCombo = new QComboBox();
	m_pCullModeCombo->addItems({"Back", "Front", "None"});
	pForm->addRow("Cull mode:", m_pCullModeCombo);

	m_pDepthModeCombo = new QComboBox();
	m_pDepthModeCombo->addItems({"Enabled", "Disabled", "ReadOnly"});
	pForm->addRow("Depth mode:", m_pDepthModeCombo);

	pMainLayout->addLayout(pForm);

	auto* pButtonRow = new QHBoxLayout();
	auto* pCreateButton = new QPushButton("Create");
	auto* pCancelButton = new QPushButton("Cancel");
	pCreateButton->setDefault(true);
	connect(pCreateButton, &QPushButton::clicked, this, &CMaterialCreatorDialog::OnCreateClicked);
	connect(pCancelButton, &QPushButton::clicked, this, &CMaterialCreatorDialog::reject);
	pButtonRow->addStretch();
	pButtonRow->addWidget(pCancelButton);
	pButtonRow->addWidget(pCreateButton);
	pMainLayout->addLayout(pButtonRow);
}

void CMaterialCreatorDialog::OnBrowseTexture() {
	QString strPath = QFileDialog::getOpenFileName(this, "Select diffuse texture (.fnttex)",
		m_strTargetDir, "Textures (*.fnttex)");
	if (strPath.isEmpty()) return;

	QString strProjectDir = CSettingsManager::GetInstance().GetProjectDir();
	QDir ProjectDir(strProjectDir);
	m_pTextureEdit->setText(ProjectDir.relativeFilePath(strPath));
}

void CMaterialCreatorDialog::OnCreateClicked() {
	QString strName = m_pNameEdit->text();
	if (strName.isEmpty()) {
		QMessageBox::warning(this, "Error", "Material name cannot be empty.");
		return;
	}
	if (m_pTextureEdit->text().isEmpty()) {
		QMessageBox::warning(this, "Error", "Please select a diffuse texture.");
		return;
	}

	QString strTxtPath = m_strTargetDir + "/" + strName + ".txt";
	QString strOutPath = m_strTargetDir + "/" + strName + ".fntmat";

	QFile TxtFile(strTxtPath);
	if (!TxtFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "Error", "Failed to create temporary descriptor file.");
		return;
	}

	QTextStream Out(&TxtFile);
	Out << "$DiffuseTexture " << m_pTextureEdit->text() << "\n";
	Out << "$Ambient " << m_pAmbientR->value() << " " << m_pAmbientG->value() << " " << m_pAmbientB->value() << "\n";
	Out << "$Diffuse " << m_pDiffuseR->value() << " " << m_pDiffuseG->value() << " " << m_pDiffuseB->value() << "\n";
	Out << "$Specular " << m_pSpecularR->value() << " " << m_pSpecularG->value() << " " << m_pSpecularB->value() << "\n";
	Out << "$Shininess " << m_pShininess->value() << "\n";
	Out << "$Opacity " << m_pOpacity->value() << "\n";
	Out << "$BlendMode " << m_pBlendModeCombo->currentText() << "\n";
	Out << "$CullMode " << m_pCullModeCombo->currentText() << "\n";
	Out << "$DepthMode " << m_pDepthModeCombo->currentText() << "\n";
	TxtFile.close();

	QString strExePath = CSettingsManager::GetInstance().GetConvertersDir() + "/fntmat";

	int nExitCode = QProcess::execute(strExePath, { strTxtPath, strOutPath });

	QFile::remove(strTxtPath);

	if (nExitCode != 0) {
		QMessageBox::warning(this, "Error", "Failed to bake material (converter exit code: " + QString::number(nExitCode) + ").");
		return;
	}

	QMessageBox::information(this, "Success", "Material created: " + strName + ".fntmat");
	accept();
}