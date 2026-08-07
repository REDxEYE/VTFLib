/*
 * VTFEdit
 * Copyright (C) 2005-2026 ficool2, Neil Jedrzejewski & Ryan Gregg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "VmtEditorOptionsDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace VTFEdit
{
	namespace
	{
		const char *const PreviewText =
			"\"LightmappedGeneric\"\n"
			"{\n"
			"\t\"$basetexture\" \"brick/brickwall001a\"\n"
			"\t\"$surfaceprop\" \"brick\"\n"
			"}";
	}

	VmtEditorOptionsDialog::VmtEditorOptionsDialog(VmtEditorSettings *pSettings, QWidget *pParent)
		: QDialog(pParent)
		, m_pSettings(pSettings)
	{
		setWindowTitle(tr("VMT Editor Options"));

		QGroupBox *pFont = new QGroupBox(tr("Font:"), this);
		QFormLayout *pForm = new QFormLayout(pFont);

		m_pFontFamily = new QFontComboBox(pFont);
		m_pFontFamily->setFontFilters(QFontComboBox::MonospacedFonts);
		m_pFontFamily->setToolTip(tr("The font used by the VMT editor."));

		m_pMonospaceOnly = new QCheckBox(tr("Only list monospaced fonts"), pFont);
		m_pMonospaceOnly->setChecked(true);

		m_pFontSize = new QSpinBox(pFont);
		m_pFontSize->setRange(4, 72);
		m_pFontSize->setSuffix(tr(" pt"));
		m_pFontSize->setToolTip(tr("The point size of the VMT editor font."));

		m_pTabSize = new QSpinBox(pFont);
		m_pTabSize->setRange(1, 16);
		m_pTabSize->setSuffix(tr(" spaces"));
		m_pTabSize->setToolTip(tr("The width of a tab character, in spaces."));

		pForm->addRow(tr("Font:"), m_pFontFamily);
		pForm->addRow(QString(), m_pMonospaceOnly);
		pForm->addRow(tr("Size:"), m_pFontSize);
		pForm->addRow(tr("Tab Size:"), m_pTabSize);

		QGroupBox *pPreview = new QGroupBox(tr("Preview:"), this);
		QVBoxLayout *pPreviewLayout = new QVBoxLayout(pPreview);
		m_pPreview = new QPlainTextEdit(QString::fromLatin1(PreviewText), pPreview);
		m_pPreview->setReadOnly(true);
		m_pPreview->setLineWrapMode(QPlainTextEdit::NoWrap);
		m_pPreview->setMinimumHeight(120);
		pPreviewLayout->addWidget(m_pPreview);

		QDialogButtonBox *pButtons = new QDialogButtonBox(
			QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset, this);
		connect(pButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(pButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
		connect(pButtons->button(QDialogButtonBox::Reset), &QPushButton::clicked,
			this, &VmtEditorOptionsDialog::onResetClicked);

		QVBoxLayout *pLayout = new QVBoxLayout(this);
		pLayout->addWidget(pFont);
		pLayout->addWidget(pPreview);
		pLayout->addWidget(pButtons);

		connect(m_pFontFamily, &QFontComboBox::currentFontChanged, this, &VmtEditorOptionsDialog::updatePreview);
		connect(m_pFontSize, &QSpinBox::valueChanged, this, &VmtEditorOptionsDialog::updatePreview);
		connect(m_pTabSize, &QSpinBox::valueChanged, this, &VmtEditorOptionsDialog::updatePreview);
		connect(m_pMonospaceOnly, &QCheckBox::toggled, this, [this](bool bChecked)
		{
			const QFont Current = m_pFontFamily->currentFont();
			m_pFontFamily->setFontFilters(bChecked
				? QFontComboBox::MonospacedFonts : QFontComboBox::AllFonts);
			m_pFontFamily->setCurrentFont(Current);
		});
	}

	int VmtEditorOptionsDialog::exec()
	{
		settingsToControls(*m_pSettings);

		const int iResult = QDialog::exec();
		if(iResult == QDialog::Accepted)
		{
			controlsToSettings();
		}

		return iResult;
	}

	void VmtEditorOptionsDialog::onResetClicked()
	{
		settingsToControls(VmtEditorSettings());
	}

	void VmtEditorOptionsDialog::updatePreview()
	{
		QFont Font = m_pFontFamily->currentFont();
		Font.setPointSize(m_pFontSize->value());
		Font.setFixedPitch(true);

		m_pPreview->setFont(Font);
		m_pPreview->setTabStopDistance(
			m_pTabSize->value() * m_pPreview->fontMetrics().horizontalAdvance(QLatin1Char(' ')));
	}

	void VmtEditorOptionsDialog::settingsToControls(const VmtEditorSettings &Settings)
	{
		const bool bMonospace = QFontDatabase::isFixedPitch(Settings.sFontFamily);
		m_pMonospaceOnly->setChecked(bMonospace);
		m_pFontFamily->setFontFilters(bMonospace
			? QFontComboBox::MonospacedFonts : QFontComboBox::AllFonts);
		m_pFontFamily->setCurrentFont(QFont(Settings.sFontFamily));

		m_pFontSize->setValue(Settings.iFontSize);
		m_pTabSize->setValue(Settings.iTabSize);

		updatePreview();
	}

	void VmtEditorOptionsDialog::controlsToSettings()
	{
		m_pSettings->sFontFamily = m_pFontFamily->currentFont().family();
		m_pSettings->iFontSize = m_pFontSize->value();
		m_pSettings->iTabSize = m_pTabSize->value();
	}
}
