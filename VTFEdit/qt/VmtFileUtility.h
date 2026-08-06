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

#include <QColor>
#include <QString>
#include <QSyntaxHighlighter>

namespace VTFEdit
{
	namespace VmtFileUtility
	{
		// Turns an absolute path such as C:\...\materials\foo\bar.vtf into foo/bar
		QString GetTexturePathFromSystemPath(const QString &sTexture);

		bool CreateDefaultMaterial(const QString &sVTFFile, const QString &sShader, bool bHasAlpha = false);
	}

	namespace VmtColors
	{
		const QColor Background(29, 31, 33);
		const QColor Text(197, 200, 198);
		const QColor Comment(181, 189, 104);
		const QColor Punctuation(197, 200, 198);
		const QColor KeyDollar(222, 147, 144);
		const QColor KeyPercent(138, 190, 183);
		const QColor Key(204, 102, 102);
		const QColor Value(129, 162, 190);
		const QColor ErrorLine(90, 40, 40);
	}

	class VmtHighlighter : public QSyntaxHighlighter
	{
		Q_OBJECT

	public:
		explicit VmtHighlighter(QTextDocument *pDocument);

	protected:
		void highlightBlock(const QString &sText) override;
	};
}
