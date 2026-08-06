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

#include "BatchConvertSettings.h"
#include "VtfOptions.h"

#include <QColor>
#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QProgressBar;
class QRadioButton;
class QTextEdit;

namespace VTFEdit
{
	class VtfOptionsDialog;

	class BatchConvertDialog : public QDialog
	{
		Q_OBJECT

	public:
		BatchConvertDialog(VtfOptions *pOptions, BatchConvertSettings *pSettings, QWidget *pParent = nullptr);

		int exec();

	private slots:
		void onBrowseInputFolder();
		void onBrowseOutputFolder();
		void onOptions();
		void onConvert();

	private:
		int countFiles(const QString &sFolder, const QStringList &sFilters, bool bRecursive) const;
		void convertFolder(const QString &sInputFolder, const QString &sOutputFolder,
			const QStringList &sFilters, bool bRecursive, SVTFCreateOptions &VTFCreateOptions);

		void log(const QString &sMessage, const QColor &Color);

		void settingsToControls();
		void controlsToSettings();

		VtfOptions *m_pOptions;
		BatchConvertSettings *m_pSettings;
		VtfOptionsDialog *m_pOptionsDialog;

		QLineEdit *m_pInputFolder;
		QLineEdit *m_pOutputFolder;
		QRadioButton *m_pToVTF;
		QRadioButton *m_pFromVTF;
		QLineEdit *m_pToVTFFilter;
		QLineEdit *m_pFromVTFFilter;
		QComboBox *m_pFromVTFFormat;
		QCheckBox *m_pRecursive;
		QCheckBox *m_pCreateVMTFiles;
		QProgressBar *m_pProgress;
		QTextEdit *m_pLog;
	};
}
