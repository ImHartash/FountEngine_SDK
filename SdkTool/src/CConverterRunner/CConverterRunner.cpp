#include "CConverterRunner.hpp"

CConverterRunner::CConverterRunner(QObject* pParent) {
    m_pProcess = new QProcess(this);
    m_pProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_pProcess, &QProcess::readyReadStandardOutput, this, &CConverterRunner::OnReadyReadStdout);
    connect(m_pProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &CConverterRunner::OnProcessFinished);
    connect(m_pProcess, &QProcess::errorOccurred, this, &CConverterRunner::OnErrorOccurred);
}

void CConverterRunner::RunConversion(const QString& strExePath, const QStringList& ListArgs) {
    if (m_pProcess->state() != QProcess::NotRunning) {
        emit OutputLine("[ERROR] Converter is already running.");
        return;
    }

    m_pProcess->start(strExePath, ListArgs);
}

bool CConverterRunner::IsRunning() const {
	return m_pProcess->state() != QProcess::NotRunning;
}

void CConverterRunner::OnReadyReadStdout() {
	emit OutputLine(QString::fromLocal8Bit(m_pProcess->readAllStandardOutput()));
}

void CConverterRunner::OnProcessFinished(int nExitCode, QProcess::ExitStatus Status) {
	emit Finished(nExitCode);
}

void CConverterRunner::OnErrorOccurred(QProcess::ProcessError Error) {
    emit OutputLine("[Error] Failed to start process: " + m_pProcess->errorString());
    emit Finished(-1);
}