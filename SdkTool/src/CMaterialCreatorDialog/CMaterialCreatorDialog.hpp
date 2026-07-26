#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>

class CMaterialCreatorDialog : public QDialog {
    Q_OBJECT

public:
    explicit CMaterialCreatorDialog(const QString& strTargetDir, QWidget* pParent = nullptr);

private slots:
    void OnBrowseTexture();
    void OnCreateClicked();

private:
    void SetupUi();
    QDoubleSpinBox* MakeColorSpinBox();

    QString m_strTargetDir;

	QLineEdit* m_pNameEdit;
	QLineEdit* m_pTextureEdit;

	QDoubleSpinBox* m_pAmbientR;
	QDoubleSpinBox* m_pAmbientG;
	QDoubleSpinBox* m_pAmbientB;

	QDoubleSpinBox* m_pDiffuseR;
	QDoubleSpinBox* m_pDiffuseG;
	QDoubleSpinBox* m_pDiffuseB;

	QDoubleSpinBox* m_pSpecularR;
	QDoubleSpinBox* m_pSpecularG;
	QDoubleSpinBox* m_pSpecularB;

	QDoubleSpinBox* m_pShininess;
	QDoubleSpinBox* m_pOpacity;

	QComboBox* m_pBlendModeCombo;
	QComboBox* m_pCullModeCombo;
	QComboBox* m_pDepthModeCombo;
};