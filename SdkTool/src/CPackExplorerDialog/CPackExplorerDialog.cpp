#include "CPackExplorerDialog.hpp"
#include "../CFntFormats.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>

CPackExplorerDialog::CPackExplorerDialog(const QString& strPackPath, QWidget* pParent)
    : QDialog(pParent), m_strPackPath(strPackPath) {
    
    setWindowTitle("Package " + QFileInfo(strPackPath).fileName());
    resize(400, 350);

    SetupUi();

    if (!LoadPackage()) {
        QMessageBox::warning(this, "Error", "Failed to read package - invalid or corrupted file.");
    }
    RefreshList();
}

void CPackExplorerDialog::SetupUi() {
    auto* pMainLayout = new QVBoxLayout(this);

    pMainLayout->addWidget(new QLabel("Contents:"));

    m_pListWidget = new QListWidget();
	m_pListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	pMainLayout->addWidget(m_pListWidget, 1);

	auto* pButtonRow = new QHBoxLayout();
	auto* pAddButton = new QPushButton("Add File...");
	auto* pRemoveButton = new QPushButton("Remove Selected");
	connect(pAddButton, &QPushButton::clicked, this, &CPackExplorerDialog::OnAddFileClicked);
	connect(pRemoveButton, &QPushButton::clicked, this, &CPackExplorerDialog::OnRemoveClicked);
	pButtonRow->addWidget(pAddButton);
	pButtonRow->addWidget(pRemoveButton);
	pMainLayout->addLayout(pButtonRow);

	auto* pBottomRow = new QHBoxLayout();
	auto* pSaveButton = new QPushButton("Save");
	auto* pCloseButton = new QPushButton("Close");
	connect(pSaveButton, &QPushButton::clicked, this, &CPackExplorerDialog::OnSaveClicked);
	connect(pCloseButton, &QPushButton::clicked, this, &CPackExplorerDialog::reject);
	pBottomRow->addStretch();
	pBottomRow->addWidget(pCloseButton);
	pBottomRow->addWidget(pSaveButton);
	pMainLayout->addLayout(pBottomRow);
}

bool CPackExplorerDialog::LoadPackage() {
	QFile File(m_strPackPath);
	if (!File.open(QIODevice::ReadOnly)) return false;

	QByteArray RawData = File.readAll();
	if (RawData.size() < static_cast<int>(sizeof(FNTPK_HEADER))) return false;

	FNTPK_HEADER Header{};
	memcpy(&Header, RawData.constData(), sizeof(FNTPK_HEADER));

	if (Header.nMagic != 0x50544E46) return false;

	int nBlocksOffset = sizeof(FNTPK_HEADER);
	int nBlocksBytes = sizeof(FNTPK_BLOCK) * Header.nBlocksCount;
	if (RawData.size() < nBlocksOffset + nBlocksBytes) return false;

	m_vecEntries.clear();
	for (uint32_t i = 0; i < Header.nBlocksCount; ++i) {
		FNTPK_BLOCK Block{};
		memcpy(&Block, RawData.constData() + nBlocksOffset + i * sizeof(FNTPK_BLOCK), sizeof(FNTPK_BLOCK));

		PackEntry_t Entry;
		Entry.strName = QString::fromLocal8Bit(Block.szResourceName);
		Entry.Data = RawData.mid(Block.nBlockDataOffset, Block.nBlockDataSize);
		m_vecEntries.push_back(Entry);
	}

	return true;
}

bool CPackExplorerDialog::SavePackage() {
	FNTPK_HEADER Header{};
	Header.nMagic = 0x50544E46;
	Header.nVersion = 1;
	Header.nFlags = 0;
	Header.nCRC = 0;
	Header.nBlocksCount = static_cast<uint32_t>(m_vecEntries.size());

	QVector<FNTPK_BLOCK> vecBlocks(m_vecEntries.size());
	uint32_t nCurrentOffset = sizeof(FNTPK_HEADER) + sizeof(FNTPK_BLOCK) * Header.nBlocksCount;

	for (int i = 0; i < m_vecEntries.size(); ++i) {
		FNTPK_BLOCK Block{};
		memset(Block.szResourceName, 0, sizeof(Block.szResourceName));
		QByteArray NameBytes = m_vecEntries[i].strName.toLocal8Bit();
		memcpy(Block.szResourceName, NameBytes.constData(),
			std::min<int>(NameBytes.size(), sizeof(Block.szResourceName) - 1));

		Block.nBlockDataOffset = nCurrentOffset;
		Block.nBlockDataSize = static_cast<uint32_t>(m_vecEntries[i].Data.size());
		nCurrentOffset += Block.nBlockDataSize;

		vecBlocks[i] = Block;
	}

	QFile File(m_strPackPath);
	if (!File.open(QIODevice::WriteOnly)) return false;

	File.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	File.write(reinterpret_cast<const char*>(vecBlocks.constData()), sizeof(FNTPK_BLOCK) * vecBlocks.size());

	for (const PackEntry_t& Entry : m_vecEntries)
		File.write(Entry.Data);

	return true;
}

void CPackExplorerDialog::RefreshList() {
	m_pListWidget->clear();
	for (const PackEntry_t& Entry : m_vecEntries) {
		auto* pItem = new QListWidgetItem(Entry.strName + "  (" + QString::number(Entry.Data.size()) + " bytes)");
		m_pListWidget->addItem(pItem);
	}
}

void CPackExplorerDialog::OnAddFileClicked() {
	QStringList listPaths = QFileDialog::getOpenFileNames(this, "Add files to package");
	for (const QString& strPath : listPaths) {
		QFile File(strPath);
		if (!File.open(QIODevice::ReadOnly)) continue;

		PackEntry_t Entry;
		Entry.strName = QFileInfo(strPath).fileName();
		Entry.Data = File.readAll();
		m_vecEntries.push_back(Entry);
	}
	RefreshList();
}

void CPackExplorerDialog::OnRemoveClicked() {
	QList<QListWidgetItem*> listSelected = m_pListWidget->selectedItems();
	QSet<int> SetRowsToRemove;
	for (QListWidgetItem* pItem : listSelected)
		SetRowsToRemove.insert(m_pListWidget->row(pItem));

	for (int i = m_vecEntries.size() - 1; i >= 0; --i)
		if (SetRowsToRemove.contains(i))
			m_vecEntries.remove(i);

	RefreshList();
}

void CPackExplorerDialog::OnSaveClicked() {
	if (SavePackage()) {
		QMessageBox::information(this, "Saved", "Package saved successfully.");
	} else {
		QMessageBox::warning(this, "Error", "Failed to save package.");
	}
}