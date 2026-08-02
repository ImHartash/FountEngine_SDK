#pragma once
#include <QFileIconProvider>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QIcon>
#include <QFileInfo>

class CFileIconProvider : public QFileIconProvider {
public:
	QIcon icon(const QFileInfo& Info) const override {
		if (Info.isDir())
			return MakeFolderIcon("#777777");

		QString strExt = Info.suffix().toLower();

		if (strExt == "fntmdl" || strExt == "obj")
			return MakeCubeIcon(strExt == "fntmdl" ? "#4a9eff" : "#777777");

		if (strExt == "fnttex" || strExt == "png" || strExt == "jpg" || strExt == "jpeg")
			return MakeImageIcon(strExt == "fnttex" ? "#4a9eff" : "#777777");

		if (strExt == "fntmat")
			return MakeMaterialIcon("#4a9eff");

		if (strExt == "fntpk")
			return MakeArchiveIcon("#4a9eff");

		return QFileIconProvider::icon(Info);
	}

	QIcon icon(IconType Type) const override {
		if (Type == QFileIconProvider::Folder)
			return MakeFolderIcon("#a08cff");
		return QFileIconProvider::icon(Type);
	}

	static QPixmap MakeCanvas(int nSize = 32) {
		QPixmap Pixmap(nSize, nSize);
		Pixmap.fill(Qt::transparent);
		return Pixmap;
	}

	static QIcon MakeCubeIcon(const QString& strColor) {
		QPixmap Pixmap = MakeCanvas();
		QPainter Painter(&Pixmap);
		Painter.setRenderHint(QPainter::Antialiasing);
		QPen Pen(QColor(strColor), 1.6);
		Painter.setPen(Pen);
		Painter.setBrush(Qt::NoBrush);

		QPolygonF Top;
		Top << QPointF(16, 4) << QPointF(27, 10) << QPointF(16, 16) << QPointF(5, 10);
		QPolygonF Left;
		Left << QPointF(5, 10) << QPointF(16, 16) << QPointF(16, 28) << QPointF(5, 22);
		QPolygonF Right;
		Right << QPointF(27, 10) << QPointF(16, 16) << QPointF(16, 28) << QPointF(27, 22);

		Painter.drawPolygon(Top);
		Painter.drawPolygon(Left);
		Painter.drawPolygon(Right);
		return QIcon(Pixmap);
	}

	static QIcon MakeImageIcon(const QString& strColor) {
		QPixmap Pixmap = MakeCanvas();
		QPainter Painter(&Pixmap);
		Painter.setRenderHint(QPainter::Antialiasing);
		QPen Pen(QColor(strColor), 1.6);
		Painter.setPen(Pen);
		Painter.setBrush(Qt::NoBrush);

		Painter.drawRoundedRect(QRectF(5, 6, 22, 20), 3, 3);
		Painter.drawEllipse(QPointF(12, 13), 2, 2);
		QPolygonF Mountain;
		Mountain << QPointF(7, 24) << QPointF(14, 15) << QPointF(19, 20) << QPointF(23, 16) << QPointF(25, 24);
		Painter.drawPolyline(Mountain);
		return QIcon(Pixmap);
	}

	static QIcon MakeMaterialIcon(const QString& strColor) {
		QPixmap Pixmap = MakeCanvas();
		QPainter Painter(&Pixmap);
		Painter.setRenderHint(QPainter::Antialiasing);
		Painter.setPen(QPen(QColor(strColor), 1.6));
		Painter.setBrush(Qt::NoBrush);
		Painter.drawEllipse(QPointF(16, 16), 11, 11);
		Painter.drawEllipse(QPointF(16, 16), 5, 5);
		return QIcon(Pixmap);
	}

	static QIcon MakeArchiveIcon(const QString& strColor) {
		QPixmap Pixmap = MakeCanvas();
		QPainter Painter(&Pixmap);
		Painter.setRenderHint(QPainter::Antialiasing);
		Painter.setPen(QPen(QColor(strColor), 1.6));
		Painter.setBrush(Qt::NoBrush);
		Painter.drawRoundedRect(QRectF(5, 6, 22, 20), 3, 3);
		Painter.drawLine(QPointF(5, 13), QPointF(27, 13));
		Painter.drawRoundedRect(QRectF(13, 16, 6, 4), 1, 1);
		return QIcon(Pixmap);
	}

	static QIcon MakeFolderIcon(const QString& strColor) {
		QPixmap Pixmap = MakeCanvas();
		QPainter Painter(&Pixmap);
		Painter.setRenderHint(QPainter::Antialiasing);

		QPen Pen(QColor(strColor), 1.6);
		Pen.setJoinStyle(Qt::RoundJoin);
		Painter.setPen(Pen);
		Painter.setBrush(Qt::NoBrush);

		QPainterPath Path;
		Path.moveTo(4, 9);
		Path.lineTo(12, 9);
		Path.lineTo(14, 11);
		Path.lineTo(28, 11);
		Path.lineTo(28, 28);
		Path.lineTo(4, 28);
		Path.closeSubpath();
		Painter.drawPath(Path);
		return QIcon(Pixmap);
	}

	static QIcon MakeCheckIcon(const QString& strColor) {
		QPixmap Pixmap = MakeCanvas();
		QPainter Painter(&Pixmap);
		Painter.setRenderHint(QPainter::Antialiasing);
		QPen Pen(QColor(strColor), 2.0);
		Pen.setCapStyle(Qt::RoundCap);
		Pen.setJoinStyle(Qt::RoundJoin);
		Painter.setPen(Pen);
		Painter.setBrush(Qt::NoBrush);

		QPainterPath Path;
		Path.moveTo(7, 16);
		Path.lineTo(13, 22);
		Path.lineTo(25, 9);
		Painter.drawPath(Path);
		return QIcon(Pixmap);
	}

	static QIcon MakeErrorIcon(const QString& strColor) {
		QPixmap Pixmap = MakeCanvas();
		QPainter Painter(&Pixmap);
		Painter.setRenderHint(QPainter::Antialiasing);
		QPen Pen(QColor(strColor), 2.0);
		Pen.setCapStyle(Qt::RoundCap);
		Painter.setPen(Pen);
		Painter.setBrush(Qt::NoBrush);

		Painter.drawLine(QPointF(9, 9), QPointF(23, 23));
		Painter.drawLine(QPointF(23, 9), QPointF(9, 23));
		return QIcon(Pixmap);
	}
};