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

#pragma once

#include "VmtEditorSettings.h"
#include "VmtTextEdit.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QSpinBox;

namespace VTFEdit
{
	class VmtHighlighter;

	class VmtEditorOptionsDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit VmtEditorOptionsDialog(VmtEditorSettings *pSettings, QWidget *pParent = nullptr);

		int exec();

	private slots:
		void onResetClicked();
		void updatePreview();

	private:
		void settingsToControls(const VmtEditorSettings &Settings);
		void controlsToSettings();

		VmtEditorSettings *m_pSettings;

		QFontComboBox *m_pFontFamily;
		QCheckBox *m_pMonospaceOnly;
		QSpinBox *m_pFontSize;
		QSpinBox *m_pTabSize;
		QComboBox *m_pTheme;
		VmtTextEdit *m_pPreview;
		VmtHighlighter *m_pPreviewHighlighter;
	};
}
