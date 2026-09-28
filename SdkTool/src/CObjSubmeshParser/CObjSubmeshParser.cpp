#include "CObjSubmeshParser.hpp"
#include <QFile>
#include <QVector3D>
#include <QVector2D>
#include <QTextStream>
#include <QRegularExpression>
#include <cmath>

static QVector<QString> SplitLine(const QString& strLine) {
	QVector<QString> strTokens = strLine.split(QRegularExpression("[ \\t]+"), Qt::SkipEmptyParts);
	return strTokens;
}

bool CObjSubmeshParser::Parse(const QString& strPath, QVector<ObjSubmesh_t>& vecSubmeshes) {
	QFile File(strPath);
	if (!File.open(QIODevice::ReadOnly)) return false;
	QTextStream In(&File);

	struct FV_t { int p, t, n; };
	struct Face_t { QVector<FV_t> vecVerts; QString strMaterial; };

	QVector<QVector3D> vecPositions;
	QVector<QVector2D> vecTexcoords;
	QVector<QVector3D> vecNormals;

	QVector<Face_t> vecFaces;
	QString strCurrentMaterial;

	while (!In.atEnd()) {
		QString strLine = In.readLine().trimmed();
		if (strLine.isEmpty() || strLine.startsWith("#")) continue;

		QVector<QString> strTokens = SplitLine(strLine);
		if (strTokens.isEmpty()) continue;

		const QString& strCommand = strTokens[0];
		if (strCommand == "v" && strTokens.size() >= 4)
			vecPositions.push_back(QVector3D(strTokens[1].toFloat(), strTokens[2].toFloat(), strTokens[3].toFloat()));
		else if (strCommand == "vt" && strTokens.size() >= 3)
			vecTexcoords.push_back(QVector2D(strTokens[1].toFloat(), 1.f - strTokens[2].toFloat()));
		else if (strCommand == "vn" && strTokens.size() >= 4)
			vecNormals.push_back(QVector3D(strTokens[1].toFloat(), strTokens[2].toFloat(), strTokens[3].toFloat()));
		else if (strCommand == "usemtl" && strTokens.size() >= 2) {
			strCurrentMaterial = strTokens[1];
		}
		else if (strCommand == "f") {
			Face_t Face;
			for (int i = 1; i < strTokens.size(); ++i) {
				FV_t FV{ 0, 0, 0 };
				QStringList strParts = strTokens[i].split('/');
				if (!strParts[0].isEmpty()) FV.p = strParts[0].toInt();
				if (strParts.size() >= 2 && !strParts[1].isEmpty()) FV.t = strParts[1].toInt();
				if (strParts.size() >= 3 && !strParts[2].isEmpty()) FV.n = strParts[2].toInt();
				Face.vecVerts.push_back(FV);
			}
			Face.strMaterial = strCurrentMaterial;
			if (Face.vecVerts.size() >= 3) vecFaces.push_back(Face);
		}
	}

	if (vecFaces.isEmpty() || vecPositions.isEmpty()) return false;

	bool bHasNormals = !vecNormals.isEmpty();

	// ===========

	QVector<QVector<Face_t>> vecGroups;
	QVector<QString> vecGroupNames;
	for (const Face_t& F : vecFaces) {
		int nIndex = vecGroupNames.indexOf(F.strMaterial);
		if (nIndex < 0) {
			vecGroupNames.push_back(F.strMaterial);
			vecGroups.push_back({ F });
		}
		else {
			vecGroups[nIndex].push_back(F);
		}
	}

	// ===========
	// TODO: Maybe in future i'll make the triangulation. 
	// But, just do not forget to click 'Triangulate' in blender.

	for (int g = 0; g < vecGroups.size(); ++g) {
		ObjSubmesh_t Submesh;
		Submesh.strMaterial = vecGroupNames[g];

		for (const Face_t& Face : vecGroups[g]) {
			if (Face.vecVerts.size() != 3) {
				// Not a triangle
				continue;
			}

			int nBase = Submesh.vecVertices.size();

			for (const FV_t& FV : Face.vecVerts) {
				PreviewVertex_t Vertex{};
				Vertex.px = vecPositions[FV.p - 1].x();
				Vertex.py = vecPositions[FV.p - 1].y();
				Vertex.pz = vecPositions[FV.p - 1].z();

				if (bHasNormals && FV.n > 0 && FV.n - 1 < vecNormals.size()) {
					Vertex.nx = vecNormals[FV.n - 1].x();
					Vertex.ny = vecNormals[FV.n - 1].y();
					Vertex.nz = vecNormals[FV.n - 1].z();
				}
				else {
					Vertex.nx = 0; Vertex.ny = 1; Vertex.nz = 0;
				}

				if (FV.t > 0 && FV.t - 1 < vecTexcoords.size()) {
					Vertex.tu = vecTexcoords[FV.t - 1].x();
					Vertex.tv = vecTexcoords[FV.t - 1].y();
				}

				Submesh.vecVertices.push_back(Vertex);
			}

			Submesh.vecIndices.push_back(nBase);
			Submesh.vecIndices.push_back(nBase+1);
			Submesh.vecIndices.push_back(nBase+2);
		}

		qDebug() << "[Parser] submesh '" << Submesh.strMaterial
			<< "' verts:" << Submesh.vecVertices.size()
			<< " idx:" << Submesh.vecIndices.size();

		if (!Submesh.vecVertices.isEmpty())
			vecSubmeshes.push_back(Submesh);
	}

	return !vecSubmeshes.isEmpty();
}