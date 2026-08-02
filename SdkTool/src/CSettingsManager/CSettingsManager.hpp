#pragma once
#include <QSettings>
#include <QString>

class CSettingsManager {
public:
    static CSettingsManager& GetInstance() {
        static CSettingsManager Instance;
        return Instance; 
    }

    QString GetConvertersDir() const {
		return m_Settings.value("ConvertersDir", "").toString();
	}
	void SetConvertersDir(const QString& strPath) {
		m_Settings.setValue("ConvertersDir", strPath);
	}

	QString GetProjectDir() const {
		return m_Settings.value("ProjectDir", "").toString();
	}
	void SetProjectDir(const QString& strPath) {
		m_Settings.setValue("ProjectDir", strPath);
	}

	QString GetPackOutputName() const {
		return m_Settings.value("PackOutputName", "package.fntpk").toString();
	}
	void SetPackOutputName(const QString& strName) {
		m_Settings.setValue("PackOutputName", strName);
	}

	QString GetPackOutputDir() const {
		QString strSaved = m_Settings.value("PackOutputDir", "").toString();
		return strSaved.isEmpty() ? GetProjectDir() : strSaved;
	}
	void SetPackOutputDir(const QString& strPackOutputDir) {
		m_Settings.setValue("PackOutputDir", strPackOutputDir);
	}

private:
	CSettingsManager() : m_Settings("FountEngine", "SdkTool") {}
	QSettings m_Settings;
};