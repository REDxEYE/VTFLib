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

#include "VTFLibQt.h"

#include <QDialog>
#include <QMap>
#include <QStringList>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QLineEdit;
class QPushButton;

namespace VTFEdit
{
	class VmtCreateDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit VmtCreateDialog(QWidget *pParent = nullptr);

		void setFromTexture(const QString &sFileName, VTFLib::CVTFFile &VTFFile);

	private slots:
		void onBrowseTexture();
		void onClear();
		void onCreate();

	private:
		QWidget *createTexturesTab();
		QWidget *createOptionsTab();

		QLineEdit *addTextureRow(QFormLayout *pForm, const QString &sLabel, const QString &sToolTip);

		// Keyed by the VMT parameter each field writes like "$basetexture".
		QMap<QString, QLineEdit *> m_Textures;
		QMap<QString, QCheckBox *> m_Options;

		QComboBox *m_pShader;
		QComboBox *m_pSurface1;
		QComboBox *m_pSurface2;
		QLineEdit *m_pKeywords;
		QPushButton *m_pCreateButton;

		// Order the texture parameters are written in.
		QStringList m_TextureOrder;
		QStringList m_OptionOrder;
	};
}
