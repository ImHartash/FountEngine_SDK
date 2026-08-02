#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
	QApplication Application(argc, argv);

	Application.setStyle(QStyleFactory::create("Fusion"));
	Application.setStyleSheet(R"(
QMainWindow, QDialog {
	background-color: #1a1a1a;
}

QWidget {
	color: #e8e8e8;
	font-family: "Segoe UI", "Ubuntu", sans-serif;
	font-size: 13px;
}

QWidget#TreeContainer {
	background-color: #212121;
	border: 1px solid #333333;
}

QWidget#TreeHeader {
	border-bottom: 1px solid #333333;
}

QLabel#TreeHeaderLabel {
	color: #cccccc;
	font-size: 13px;
}

QTreeView {
	background-color: transparent;
	border: none;
	padding: 6px;
	outline: none;
}

QTreeView::item {
	padding: 5px 8px;
	height: 22px;
}

QTreeView::item:selected {
	background-color: #152b44;
	color: #ffffff;
}

QWidget#DropZone {
	border: 1px dashed #444444;
	margin: 8px;
}

QLabel#DropZoneIcon {
	color: #666666;
	font-size: 16px;
}

QLabel#DropZoneText {
	color: #666666;
	font-size: 11px;
}

QWidget#PreviewCard {
	background-color: #212121;
	border: 1px solid #333333;
}

QLabel#PreviewNameLabel {
	font-size: 14px;
	font-weight: 600;
	color: #e8e8e8;
}

QLabel#PreviewMetaLabel {
	font-size: 12px;
	color: #888888;
}

QPushButton {
	background-color: transparent;
	border: 2px solid #4a9eff;
	border-radius: 8px;
	color: #4a9eff;
	padding: 6px 16px;
	font-weight: 500;
}

QPushButton:hover {
	background-color: rgba(74, 158, 255, 0.15);
}

QPushButton:pressed {
	background-color: rgba(74, 158, 255, 0.3);
}

QPushButton:disabled {
	border-color: #444444;
	color: #666666;
}

QPushButton#PackButton {
	padding: 5px 12px;
	font-size: 12px;
}

QLabel#StatusLabel {
	color: #999999;
	font-size: 12px;
	padding: 2px 4px;
}

QPlainTextEdit {
	background-color: #151515;
	border: 1px solid #333333;
	font-family: "Consolas", "monospace";
	font-size: 12px;
	padding: 6px;
}

QScrollBar:vertical {
	background: transparent;
	width: 10px;
	margin: 0;
}

QScrollBar::handle:vertical {
	background: #444444;
	border-radius: 5px;
	min-height: 20px;
}

QScrollBar::handle:vertical:hover {
	background: #555555;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
	height: 0px;
}

QMenuBar {
	background-color: #1a1a1a;
	border-bottom: 1px solid #333333;
	padding: 2px;
}

QMenuBar::item {
	padding: 4px 10px;
}

QMenuBar::item:selected {
	background-color: #2a2a2a;
}
	)");

	QString strProjectPath = QDir::homePath() + "/Desktop/sdk_tool_test_assets";
	QDir().mkpath(strProjectPath);

	MainWindow Window(strProjectPath);
	Window.show();

	return Application.exec();
}