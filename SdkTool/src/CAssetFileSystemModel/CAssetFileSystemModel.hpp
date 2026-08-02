#pragma once
#include <QFileSystemModel>

class CAssetFileSystemModel : public QFileSystemModel {
public:
	using QFileSystemModel::QFileSystemModel;

	QVariant data(const QModelIndex& Index, int nRole) const override {
		if (nRole == Qt::DisplayRole && Index.column() == 0 && isDir(Index)) {
			return fileName(Index) + "/";
		}
		return QFileSystemModel::data(Index, nRole);
	}
};