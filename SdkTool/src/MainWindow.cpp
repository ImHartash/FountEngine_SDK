#include "MainWindow.hpp"
#include "CSettingsDialog/CSettingsDialog.hpp"
#include "CSettingsManager/CSettingsManager.hpp"
#include "CPackExplorerDialog/CPackExplorerDialog.hpp"
#include "CMaterialCreatorDialog/CMaterialCreatorDialog.hpp"
#include "CFntFormats.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QImage>
#include <QMenuBar>
#include <QFile>

MainWindow::MainWindow(const QString& strProjectPath, QWidget* pParent)
	: QMainWindow(pParent), m_strProjectPath(strProjectPath) {

	setWindowTitle("FountEngine SDK Tool");
	resize(900, 550);

	m_pRunner = new CConverterRunner(this);
	connect(m_pRunner, &CConverterRunner::OutputLine, this, &MainWindow::OnLogLine);
	connect(m_pRunner, &CConverterRunner::Finished, this, &MainWindow::OnConversionFinished);

	SetupUi();
	CheckFirstRun();
}

void MainWindow::SetupUi() {
	auto* pCentral = new QWidget(this);
	auto* pMainLayout = new QHBoxLayout(pCentral);

	// --- Left Side ---
	m_pFileSystemModel = new CAssetFileSystemModel(this);
	m_pIconProvider = new CFileIconProvider();
	m_pFileSystemModel->setIconProvider(m_pIconProvider);
	m_pFileSystemModel->setReadOnly(false);
	m_pFileSystemModel->setNameFilters({"*.obj", "*.png", "*.jpg", "*.fntmdl", "*.fnttex", "*.fntmat", "*.fntpk"});
	m_pFileSystemModel->setNameFilterDisables(false);

	m_pTreeView = new CAssetTreeView();
	m_pTreeView->setFrameShape(QFrame::NoFrame);
	m_pTreeView->setModel(m_pFileSystemModel);
	m_pTreeView->header()->hide();
	m_pTreeView->hideColumn(1);
	m_pTreeView->hideColumn(2);
	m_pTreeView->hideColumn(3);
	m_pTreeView->setAcceptDrops(true);
	m_pTreeView->setDragDropMode(QAbstractItemView::DragDrop);
	m_pTreeView->setDefaultDropAction(Qt::MoveAction);
	m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	connect(m_pTreeView, &CAssetTreeView::FilesDropped, this, &MainWindow::OnFilesDropped);
	connect(m_pTreeView, &QTreeView::doubleClicked, this, &MainWindow::OnItemDoubleClicked);
	connect(m_pTreeView, &CAssetTreeView::CreateMaterialRequest, this, [this] (const QString& strTargetDir) {
		CMaterialCreatorDialog Dialog(strTargetDir, this);
		Dialog.exec();
	});
	connect(m_pTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
		this, &MainWindow::OnSelectionChanged);

	m_pTreeContainer = new QWidget();
	m_pTreeContainer->setObjectName("TreeContainer");
	auto* pTreeContainerLayout = new QVBoxLayout(m_pTreeContainer);
	pTreeContainerLayout->setContentsMargins(0, 0, 0, 0);
	pTreeContainerLayout->setSpacing(0);

	// TreeView Header
	auto* pTreeHeader = new QWidget();
	pTreeHeader->setObjectName("TreeHeader");
	auto* pTreeHeaderLayout = new QHBoxLayout(pTreeHeader);
	pTreeHeaderLayout->setContentsMargins(10, 6, 10, 6);

	auto* pFolderIconLabel = new QLabel();
	pFolderIconLabel->setPixmap(CFileIconProvider::MakeFolderIcon("#a08cff").pixmap(16, 16));
	m_pProjectNameLabel = new QLabel(QFileInfo(m_strProjectPath).fileName() + "/");
	m_pProjectNameLabel->setObjectName("TreeHeaderLabel");

	pTreeHeaderLayout->addWidget(pFolderIconLabel);
	pTreeHeaderLayout->addWidget(m_pProjectNameLabel);
	pTreeHeaderLayout->addStretch();

	// Dropzone
	auto* pDropZone = new QWidget();
	pDropZone->setObjectName("DropZone");
	pDropZone->setAcceptDrops(true);
	pDropZone->setFixedHeight(70);
	auto* pDropZoneLayout = new QVBoxLayout(pDropZone);
	pDropZoneLayout->setAlignment(Qt::AlignCenter);
	pDropZoneLayout->setSpacing(4);

	auto* pDropIconLabel = new QLabel("⇧");
	pDropIconLabel->setAlignment(Qt::AlignCenter);
	pDropIconLabel->setObjectName("DropZoneIcon");
	auto* pDropTextLabel = new QLabel("Drag and drop file to convert");
	pDropTextLabel->setAlignment(Qt::AlignCenter);
	pDropTextLabel->setObjectName("DropZoneText");

	pDropZoneLayout->addWidget(pDropIconLabel);
	pDropZoneLayout->addWidget(pDropTextLabel);

	pTreeContainerLayout->addWidget(pTreeHeader);
	pTreeContainerLayout->addWidget(m_pTreeView, 1);
	pTreeContainerLayout->addWidget(pDropZone);

	pMainLayout->addWidget(m_pTreeContainer, 1);

	// --- Right Side ---
	auto* pRightLayout = new QVBoxLayout();

	auto* pPreviewCard = new QWidget();
	pPreviewCard->setObjectName("PreviewCard");
	auto* pPreviewCardLayout = new QVBoxLayout(pPreviewCard);
	pPreviewCardLayout->setAlignment(Qt::AlignCenter);
	pPreviewCardLayout->setSpacing(6);

	m_pPreviewIconLabel = new QLabel();
	m_pPreviewIconLabel->setAlignment(Qt::AlignCenter);
	m_pPreviewIconLabel->setFixedSize(48, 48);
	m_pPreviewIconLabel->setScaledContents(true);

	m_pPreviewNameLabel = new QLabel("No selection");
	m_pPreviewNameLabel->setAlignment(Qt::AlignCenter);
	m_pPreviewNameLabel->setObjectName("PreviewNameLabel");

	m_pPreviewMetaLabel = new QLabel("");
	m_pPreviewMetaLabel->setAlignment(Qt::AlignCenter);
	m_pPreviewMetaLabel->setObjectName("PreviewMetaLabel");

	auto* pIconWrap = new QHBoxLayout();
	pIconWrap->setAlignment(Qt::AlignCenter);
	pIconWrap->addWidget(m_pPreviewIconLabel);

	pPreviewCardLayout->addLayout(pIconWrap);
	pPreviewCardLayout->addWidget(m_pPreviewNameLabel);
	pPreviewCardLayout->addWidget(m_pPreviewMetaLabel);

	pRightLayout->addWidget(pPreviewCard, 1);

	// Status
	auto* pStatusRow = new QWidget();
	auto* pStatusLayout = new QHBoxLayout(pStatusRow);
	pStatusLayout->setContentsMargins(4, 2, 4, 2);
	pStatusLayout->setSpacing(6);

	m_pStatusIconLabel = new QLabel();
	m_pStatusIconLabel->setFixedSize(14, 14);
	m_pStatusIconLabel->setScaledContents(true);

	m_pStatusTextLabel = new QLabel("");
	m_pStatusTextLabel->setObjectName("StatusLabel");

	pStatusLayout->addWidget(m_pStatusIconLabel);
	pStatusLayout->addWidget(m_pStatusTextLabel);
	pStatusLayout->addStretch();

	pRightLayout->addWidget(pStatusRow);

	m_pLogConsole = new QPlainTextEdit();
	m_pLogConsole->setFrameShape(QFrame::NoFrame);
	m_pLogConsole->setReadOnly(true);
	m_pLogConsole->setMaximumHeight(100);
	pRightLayout->addWidget(m_pLogConsole);

	auto* pPackButton = new QPushButton("Pack to .fntpk");
	pPackButton->setObjectName("PackButton");
	pPackButton->setIcon(CFileIconProvider::MakeArchiveIcon("#4a9eff"));
	connect(pPackButton, &QPushButton::clicked, this, &MainWindow::OnPackClicked);

	auto* pPackRow = new QHBoxLayout();
	pPackRow->addStretch();
	pPackRow->addWidget(pPackButton);
	pRightLayout->addLayout(pPackRow);

	pMainLayout->addLayout(pRightLayout, 1);

	setCentralWidget(pCentral);

	// --- Menu Bar: Settings only ---
	auto* pSettingsAction = menuBar()->addAction("Settings...");
	connect(pSettingsAction, &QAction::triggered, this, [this]() {
		CSettingsDialog Dialog(this);
		if (Dialog.exec() == QDialog::Accepted) {
			ReloadProject();
		}
	});

	ReloadProject();
}

void MainWindow::OnFilesDropped(const QString& strTargetDir, const QStringList& listPaths) {
	for (const QString& strSourcePath : listPaths) {
		QFileInfo Info(strSourcePath);
		QString strExtension = Info.suffix().toLower();

		bool bNeedConversion = (strExtension == "obj" || strExtension == "png"
			|| strExtension == "jpg" || strExtension == "jpeg");

		if (bNeedConversion) {
			EnqueueConversion(strSourcePath, strTargetDir);
		} else {
			QString strDestPath = strTargetDir + "/" + Info.fileName();
			if (QFileInfo(strSourcePath).absoluteFilePath() != QFileInfo(strDestPath).absoluteFilePath())
				QFile::copy(strSourcePath, strDestPath);
		}
	}
	ProcessNextInQueue();
}

void MainWindow::EnqueueConversion(const QString& strSourcePath, const QString& strTargetDir) {
	QFileInfo Info(strSourcePath);
	QString strExt = Info.suffix().toLower();
	QString strBaseName = Info.completeBaseName();

	PendingConversion_t Conversion;
	Conversion.strDisplayName = Info.fileName();
	Conversion.strSourcePath = strSourcePath;

	QString strConvertersDir = CSettingsManager::GetInstance().GetConvertersDir();
	if (strExt == "obj") {
		Conversion.strExePath = strConvertersDir + "/fntmdl";
		Conversion.strOutputPath = strTargetDir + "/" + strBaseName + ".fntmdl";
		Conversion.listArgs = { strSourcePath, Conversion.strOutputPath };
	} else if (strExt == "png" || strExt == "jpg") {
		Conversion.strExePath = strConvertersDir + "/fnttex";
		Conversion.strOutputPath = strTargetDir + "/" + strBaseName + ".fnttex";
		Conversion.listArgs = { strSourcePath, Conversion.strOutputPath };
	} else {
		return;
	}

	m_QueuePending.enqueue(Conversion);
}

void MainWindow::ProcessNextInQueue() {
	if (m_QueuePending.isEmpty() || m_pRunner->IsRunning()) return;

	PendingConversion_t Conversion = m_QueuePending.dequeue();
	m_pStatusIconLabel->clear();
	m_pStatusTextLabel->setText("Converting: " + Conversion.strDisplayName);
	m_pStatusTextLabel->setStyleSheet("color: #999999;");
	m_pLogConsole->appendPlainText("--- " + Conversion.strDisplayName + " ---");

	m_ConversionHistory[Conversion.strOutputPath] = { QFileInfo(Conversion.strSourcePath).fileName(), QDateTime::currentDateTime() };

	m_pRunner->RunConversion(Conversion.strExePath, Conversion.listArgs);
}

void MainWindow::OnConversionFinished(int nExitCode) {
	if (nExitCode == 0) {
		m_pStatusIconLabel->setPixmap(CFileIconProvider::MakeCheckIcon("#4caf50").pixmap(14, 14));
		m_pStatusTextLabel->setText("Done.");
		m_pStatusTextLabel->setStyleSheet("color: #4caf50;");
	} else {
		m_pStatusIconLabel->setPixmap(CFileIconProvider::MakeErrorIcon("#e05252").pixmap(14, 14));
		m_pStatusTextLabel->setText("Error, exit code: " + QString::number(nExitCode));
		m_pStatusTextLabel->setStyleSheet("color: #e05252;");
	}
	ProcessNextInQueue();
}

void MainWindow::OnLogLine(const QString& strLine) {
	m_pLogConsole->appendPlainText(strLine.trimmed());
}

void MainWindow::OnSelectionChanged() {
	QModelIndex Index = m_pTreeView->currentIndex();
	if (!Index.isValid()) return;

	QString strPath = m_pFileSystemModel->filePath(Index);
	UpdateFileInfoPanel(strPath);
}

void MainWindow::UpdateFileInfoPanel(const QString& strPath) {
	QFileInfo Info(strPath);
	QString strExt = Info.suffix().toLower();

	m_pPreviewNameLabel->setText(Info.fileName());

	QIcon FileIcon = m_pIconProvider->icon(Info);
	m_pPreviewIconLabel->setPixmap(FileIcon.pixmap(48, 48));

	QString strMeta;

	if (strExt == "fntmdl") {
		QFile File(strPath);
		if (File.open(QIODevice::ReadOnly)) {
			FNTMDL_HEADER Header{};
			if (File.read(reinterpret_cast<char*>(&Header), sizeof(Header)) == sizeof(Header)) {
				strMeta = QString("%1 verts · %2 tris")
					.arg(Header.nVertexCount)
					.arg(Header.nIndexCount / 3);
			}
		}
	} else if (strExt == "fnttex") {
		QFile File(strPath);
		if (File.open(QIODevice::ReadOnly)) {
			FNTTEX_HEADER Header{};
			if (File.read(reinterpret_cast<char*>(&Header), sizeof(Header)) == sizeof(Header)) {
				strMeta = QString("%1x%2 · %3 mips").arg(Header.nWidth).arg(Header.nHeight).arg(Header.nMipLevels);
			}
		}
		QPixmap DecodedPixmap = DecodeFntTex(strPath);
		if (!DecodedPixmap.isNull()) {
			m_pPreviewIconLabel->setFixedSize(80, 80);
			m_pPreviewIconLabel->setPixmap(DecodedPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
	}

	if (strMeta.isEmpty())
		strMeta = QString("%1 KB").arg(Info.size() / 1024.0, 0, 'f', 1);

	m_pPreviewMetaLabel->setText(strMeta);

	if (m_ConversionHistory.contains(strPath)) {
		const ConversionRecord_t& Record = m_ConversionHistory[strPath];
		qint64 nSecsAgo = Record.When.secsTo(QDateTime::currentDateTime());
		QString strWhen = nSecsAgo < 60 ? "just now" : QString("%1 min ago").arg(nSecsAgo / 60);
		m_pStatusTextLabel->setText("Auto-converted from " + Record.strSourceName + " · " + strWhen);
		m_pStatusIconLabel->setPixmap(CFileIconProvider::MakeCheckIcon("#4caf50").pixmap(14, 14));
		m_pStatusTextLabel->setStyleSheet("color: #999999;");
	}

	QPixmap Pixmap(strPath);
	if (!Pixmap.isNull()) {
		m_pPreviewIconLabel->setFixedSize(80, 80);
		m_pPreviewIconLabel->setPixmap(Pixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	} else {
		m_pPreviewIconLabel->setFixedSize(48, 48);
	}
}

void MainWindow::OnPackClicked() {
	CSettingsManager& Settings = CSettingsManager::GetInstance();
	m_pLogConsole->appendPlainText("--- Packing project ---");
	m_pRunner->RunConversion(Settings.GetConvertersDir() + "/fntpk",
		{ m_strProjectPath, Settings.GetPackOutputDir() + "/" + Settings.GetPackOutputName() });
}

void MainWindow::OnItemDoubleClicked(const QModelIndex& Index) {
	QString strPath = m_pFileSystemModel->filePath(Index);
	if (QFileInfo(strPath).suffix().toLower() != "fntpk") return;

	CPackExplorerDialog Dialog(strPath, this);
	Dialog.exec();
}

void MainWindow::ReloadProject() {
	QString strNewProjectPath = CSettingsManager::GetInstance().GetProjectDir();
	if (strNewProjectPath.isEmpty()) return;

	QDir().mkpath(strNewProjectPath);
	m_strProjectPath = strNewProjectPath;

	m_pFileSystemModel->setRootPath(m_strProjectPath);
	m_pTreeView->setRootIndex(m_pFileSystemModel->index(m_strProjectPath));

	m_pProjectNameLabel->setText(QFileInfo(m_strProjectPath).fileName() + "/");
}

void MainWindow::CheckFirstRun() {
	CSettingsManager& Settings = CSettingsManager::GetInstance();

	bool bMissingConverters = Settings.GetConvertersDir().isEmpty();
	bool bMissingProject = Settings.GetProjectDir().isEmpty();

	if (!bMissingConverters && !bMissingProject) return;

	QMessageBox::information(this, "Welcome",
		"Looks like this is your first time running the tool.\n\n"
		"Please set up the converters folder and project folder before you continue.");

	CSettingsDialog Dialog(this);
	if (Dialog.exec() == QDialog::Accepted) {
		ReloadProject();
	}
}

QPixmap MainWindow::DecodeFntTex(const QString& strPath) {
	QFile File(strPath);
	if (!File.open(QIODevice::ReadOnly)) return QPixmap();

	FNTTEX_HEADER Header{};
	if (File.read(reinterpret_cast<char*>(&Header), sizeof(Header)) != sizeof(Header))
		return QPixmap();

	if (Header.nDXFormat != 28)
		return QPixmap();

	qint64 nMip0Size = static_cast<qint64>(Header.nWidth) * Header.nHeight * 4;
	QByteArray PixelData = File.read(nMip0Size);
	if (PixelData.size() != nMip0Size) return QPixmap();

	QImage Image(reinterpret_cast<const uchar*>(PixelData.constData()),
		Header.nWidth, Header.nHeight, QImage::Format_RGBA8888);

	return QPixmap::fromImage(Image.copy());
}