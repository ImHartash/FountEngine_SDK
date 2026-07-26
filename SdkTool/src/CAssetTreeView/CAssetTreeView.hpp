#pragma once
#include <QTreeView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMimeData>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

class CAssetTreeView : public QTreeView {
	Q_OBJECT

public:
	using QTreeView::QTreeView; 

signals:
	void FilesDropped(const QString& strTargetDir, const QStringList& listPaths);
	void CreateMaterialRequest(const QString& strTargetDir);

protected:
	void dragEnterEvent(QDragEnterEvent* pEvent) override {
		if (pEvent->mimeData()->hasUrls())
			pEvent->acceptProposedAction();
	}

	void dragMoveEvent(QDragMoveEvent* pEvent) override {
		if (pEvent->mimeData()->hasUrls())
			pEvent->acceptProposedAction();
	}

	void dropEvent(QDropEvent* pEvent) override {
		if (pEvent->source() == this) {
			QTreeView::dropEvent(pEvent);
			return;	
		}

		QModelIndex TargetIndex = indexAt(pEvent->position().toPoint());

		QString strTargetDir;
		auto* pFsModel = qobject_cast<QFileSystemModel*>(model());
		if (pFsModel && TargetIndex.isValid()) {
			strTargetDir = pFsModel->filePath(TargetIndex);
			if (QFileInfo(strTargetDir).isFile())
				strTargetDir = QFileInfo(strTargetDir).absolutePath();
		} else if (pFsModel) {
			strTargetDir = pFsModel->rootPath();
		}

		QStringList listPaths;
		for (const QUrl& Url : pEvent->mimeData()->urls())
			listPaths << Url.toLocalFile();

		emit FilesDropped(strTargetDir, listPaths);
		pEvent->acceptProposedAction();
	}

	void keyPressEvent(QKeyEvent* pEvent) override {
		if (pEvent->key() == Qt::Key_Delete) {
			DeleteSelected();
			return;
		}
		QTreeView::keyPressEvent(pEvent);
	}

	void contextMenuEvent(QContextMenuEvent* pEvent) override {
		auto* pFileSystemModel = qobject_cast<QFileSystemModel*>(model());
		if (!pFileSystemModel) return;

		QModelIndex Index = indexAt(pEvent->pos());

		QMenu Menu(this);
		QAction* pNewFolderAction = Menu.addAction("New Folder");
		QAction* pRenameAction = Menu.addAction("Rename");
		QAction* pDeleteAction = Menu.addAction("Delete");
		QAction* pCreateMaterialAction = Menu.addAction("New Material...");
		
		pRenameAction->setEnabled(Index.isValid());
		pDeleteAction->setEnabled(!selectionModel()->selectedIndexes().isEmpty());

		QAction* pChosen = Menu.exec(pEvent->globalPos());

		if (pChosen == pNewFolderAction) {
			CreateFolderAt(Index);	
		}
		else if (pChosen == pRenameAction && Index.isValid()) {
			edit(Index);
		}
		else if (pChosen == pDeleteAction) {
			DeleteSelected();
		}
		else if (pChosen == pCreateMaterialAction) {
			emit CreateMaterialRequest(GetTargetDirByIndex(Index));
		}
	}

private:
	QString GetTargetDirByIndex(const QModelIndex& Index) {
		auto* pFileSystemModel = qobject_cast<QFileSystemModel*>(model());
		if (!pFileSystemModel) return "";

		QString strTargetDir = pFileSystemModel->rootPath();
		if (Index.isValid()) {
			strTargetDir = pFileSystemModel->filePath(Index);
			if (QFileInfo(strTargetDir).isFile()) 
				strTargetDir = QFileInfo(strTargetDir).absolutePath();
		}

		return strTargetDir; 
	}

	void CreateFolderAt(const QModelIndex& Index) {
		QString strTargetDir = GetTargetDirByIndex(Index);
		if (strTargetDir.isEmpty()) return;

		bool bOk = false;
		QString strFolderName = QInputDialog::getText(this, "New Folder", "Folder name:", QLineEdit::Normal, "New Folder", &bOk);
		if (!bOk || strFolderName.isEmpty()) return;

		if (strFolderName.contains('/') || strFolderName.contains('\\')) {
			QMessageBox::warning(this, "Error", "Folder name cannot contain slashes.");
			return;
		}

		QDir Dir(strTargetDir);
		if (Dir.exists(strFolderName)) {
			QMessageBox::warning(this, "Error", "A folder with this name already exists.");
			return;
		}

		Dir.mkdir(strFolderName);
	}

	void DeleteSelected() {
		QModelIndexList SelectedList = selectionModel()->selectedIndexes();
		if (SelectedList.isEmpty()) return;

		auto* pFileSystemModel = qobject_cast<QFileSystemModel*>(model());
		if (!pFileSystemModel) return;

		QStringList NamesList;
		for (const QModelIndex& Index : SelectedList) {
			if (Index.column() == 0) {
				NamesList << pFileSystemModel->fileName(Index);
			}
		}

		if (NamesList.isEmpty()) return;

		auto Answer = QMessageBox::question(this, "Delete files",
			"Delete " + QString::number(NamesList.size()) + " item(s)?\n\n" + NamesList.join("\n"),
			QMessageBox::Yes | QMessageBox::No);
		
		if (Answer != QMessageBox::Yes) return;

		for (const QModelIndex& Index : SelectedList) {
			if (Index.column() != 0) continue;

			if (pFileSystemModel->isDir(Index)) {
				pFileSystemModel->rmdir(Index);
			}
			else {
				pFileSystemModel->remove(Index);
			}
		}
	}
};