#include "CSettingsDialog.hpp"
#include "../CSettingsManager/CSettingsManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>

CSettingsDialog::CSettingsDialog(QWidget* pParent) : QDialog(pParent) {
	setWindowTitle("Settings");
	resize(450, 200);
	SetupUi();
	LoadCurrentValues();
}

void CSettingsDialog::SetupUi() {
	auto* pMainLayout = new QVBoxLayout(this);

	pMainLayout->addWidget(new QLabel("Converters folder:"));
	auto* pConvertersRow = new QHBoxLayout();
	m_pConvertersDirEdit = new QLineEdit();
	auto* pConvertersBrowse = new QPushButton("Browse...");
	connect(pConvertersBrowse, &QPushButton::clicked, this, &CSettingsDialog::OnBrowseConvertersDir);
	pConvertersRow->addWidget(m_pConvertersDirEdit);
	pConvertersRow->addWidget(pConvertersBrowse);
	pMainLayout->addLayout(pConvertersRow);

	pMainLayout->addWidget(new QLabel("Project folder:"));
	auto* pProjectRow = new QHBoxLayout();
	m_pProjectDirEdit = new QLineEdit();
	auto* pProjectBrowse = new QPushButton("Browse...");
	connect(pProjectBrowse, &QPushButton::clicked, this, &CSettingsDialog::OnBrowseProjectDir);
	pProjectRow->addWidget(m_pProjectDirEdit);
	pProjectRow->addWidget(pProjectBrowse);
	pMainLayout->addLayout(pProjectRow);

	pMainLayout->addWidget(new QLabel("Pack output filename:"));
	m_pPackNameEdit = new QLineEdit();
	pMainLayout->addWidget(m_pPackNameEdit);

	pMainLayout->addWidget(new QLabel("Pack output folder:"));
	auto* pPackDirRow = new QHBoxLayout();
	m_pPackOutputDirEdit = new QLineEdit();
	m_pPackOutputDirEdit->setPlaceholderText("(default: project folder)");
	auto* pPackDirBrowse = new QPushButton("Browse...");
	connect(pPackDirBrowse, &QPushButton::clicked, this, &CSettingsDialog::OnBrowsePackOutputDir);
	pPackDirRow->addWidget(m_pPackOutputDirEdit);
	pPackDirRow->addWidget(pPackDirBrowse);
	pMainLayout->addLayout(pPackDirRow);

	auto* pButtonRow = new QHBoxLayout();
	auto* pSaveButton = new QPushButton("Save");
	auto* pCancelButton = new QPushButton("Cancel");
	connect(pSaveButton, &QPushButton::clicked, this, &CSettingsDialog::OnSaveClicked);
	connect(pCancelButton, &QPushButton::clicked, this, &CSettingsDialog::reject);
	pButtonRow->addStretch();
	pButtonRow->addWidget(pCancelButton);
	pButtonRow->addWidget(pSaveButton);
	pMainLayout->addLayout(pButtonRow);
}

void CSettingsDialog::LoadCurrentValues() {
	auto& Settings = CSettingsManager::GetInstance();
	m_pConvertersDirEdit->setText(Settings.GetConvertersDir());
	m_pProjectDirEdit->setText(Settings.GetProjectDir());
	m_pPackNameEdit->setText(Settings.GetPackOutputName());
	m_pPackOutputDirEdit->setText(Settings.GetPackOutputDir());
}

void CSettingsDialog::OnBrowseConvertersDir() {
	QString strDir = QFileDialog::getExistingDirectory(this, "Select converters folder");
	if (!strDir.isEmpty()) m_pConvertersDirEdit->setText(strDir);
}

void CSettingsDialog::OnBrowseProjectDir() {
	QString strDir = QFileDialog::getExistingDirectory(this, "Select project folder");
	if (!strDir.isEmpty()) m_pProjectDirEdit->setText(strDir);
}

void CSettingsDialog::OnBrowsePackOutputDir() {
	QString strDir = QFileDialog::getExistingDirectory(this, "Select pack output folder");
	if (!strDir.isEmpty()) m_pPackOutputDirEdit->setText(strDir);
}

void CSettingsDialog::OnSaveClicked() {
	auto& Settings = CSettingsManager::GetInstance();
	Settings.SetConvertersDir(m_pConvertersDirEdit->text());
	Settings.SetProjectDir(m_pProjectDirEdit->text());
	Settings.SetPackOutputName(m_pPackNameEdit->text());
	Settings.SetPackOutputDir(m_pPackOutputDirEdit->text());
	accept(); 
}