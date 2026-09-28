#pragma once
#include <QString>
#include <QVector>
#include "../CDX11Renderer/CDX11Renderer.hpp"

struct ObjSubmesh_t {
	QString strMaterial;
	QVector<PreviewVertex_t> vecVertices;
	QVector<uint32_t> vecIndices;
};

class CObjSubmeshParser {
public:
	static bool Parse(const QString& strPath, QVector<ObjSubmesh_t>& vecSubmeshes);
};