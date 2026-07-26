#pragma once
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class CConverterRunner : public QObject {
    Q_OBJECT

public:
    explicit CConverterRunner(QObject* pParent = nullptr);

    void RunConversion(const QString& strExePath, const QStringList& ListArgs);
    bool IsRunning() const;

signals:
    void OutputLine(const QString& strLine);
    void Finished(int nExitCode);

private slots:
    void OnReadyReadStdout();
    void OnProcessFinished(int nExitCode, QProcess::ExitStatus Status);
    void OnErrorOccurred(QProcess::ProcessError Error);

private:
    QProcess* m_pProcess;
};