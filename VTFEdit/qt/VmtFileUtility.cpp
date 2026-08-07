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

#include "VmtFileUtility.h"

#include <QByteArray>
#include <QFileInfo>

namespace VTFEdit
{
	namespace VmtFileUtility
	{
		QString GetTexturePathFromSystemPath(const QString &sPath)
		{
			if(sPath.isEmpty())
			{
				return QString();
			}

			const QString sExtension = QStringLiteral(".vtf");
			const QString sMaterials = QStringLiteral("materials");

			QString sTexture = sPath.trimmed();
			sTexture.replace(QLatin1Char('\\'), QLatin1Char('/'));
			sTexture.replace(QLatin1String("//"), QLatin1String("/"));

			if(sTexture.endsWith(sExtension, Qt::CaseInsensitive))
			{
				sTexture.chop(sExtension.length());
				sTexture = sTexture.trimmed();
			}

			// Drop any drive letter.
			const int iColon = sTexture.indexOf(QLatin1Char(':'));
			if(iColon != -1)
			{
				sTexture = sTexture.mid(iColon + 1);
			}

			// Keep only what follows the last "materials" component.
			for(int i = 0; i <= sTexture.length() - sMaterials.length(); i++)
			{
				if(sTexture.mid(i, sMaterials.length()).compare(sMaterials, Qt::CaseInsensitive) == 0)
				{
					sTexture = sTexture.mid(i + sMaterials.length());
					i = -1;	// restart the scan on the shortened string
				}
			}

			while(sTexture.startsWith(QLatin1Char('/')))
			{
				sTexture = sTexture.mid(1);
			}

			while(sTexture.endsWith(QLatin1Char('/')))
			{
				sTexture.chop(1);
			}

			return sTexture;
		}

		bool CreateDefaultMaterial(const QString &sVTFFile, const QString &sShader, bool bHasAlpha)
		{
			if(!sVTFFile.endsWith(QLatin1String(".vtf"), Qt::CaseInsensitive))
			{
				return false;
			}

			QString sVMTFile = sVTFFile;
			sVMTFile.chop(4);
			sVMTFile += QLatin1String(".vmt");

			if(QFileInfo::exists(sVMTFile))
			{
				return false;
			}

			VTFLib::CVMTFile VMTFile;

			VMTFile.Create(sShader.toLocal8Bit().constData());

			const QByteArray Texture = GetTexturePathFromSystemPath(sVTFFile).toLocal8Bit();
			VMTFile.GetRoot()->AddStringNode("$basetexture", Texture.constData());

			if(bHasAlpha)
			{
				VMTFile.GetRoot()->AddIntegerNode("$translucent", 1);
			}

			return VMTFile.Save(sVMTFile.toLocal8Bit().constData()) != vlFalse;
		}
	}

	namespace VmtColors
	{
		const Scheme &Get(bool bDark)
		{
			// Tomorrow Night
			static const Scheme Dark =
			{
				QColor(29, 31, 33),		// Background
				QColor(197, 200, 198),	// Text
				QColor(181, 189, 104),	// Comment
				QColor(197, 200, 198),	// Punctuation
				QColor(222, 147, 144),	// KeyDollar
				QColor(138, 190, 183),	// KeyPercent
				QColor(204, 102, 102),	// Key
				QColor(129, 162, 190),	// Value
				QColor(90, 40, 40),		// ErrorLine
				QColor(35, 37, 39),		// LineNumberBackground
				QColor(105, 108, 106),	// LineNumber
				QColor(197, 200, 198),	// LineNumberCurrent
			};

			// Tomorrow
			static const Scheme Light =
			{
				QColor(255, 255, 255),	// Background
				QColor(77, 77, 76),		// Text
				QColor(113, 140, 0),	// Comment
				QColor(77, 77, 76),		// Punctuation
				QColor(245, 135, 31),	// KeyDollar
				QColor(62, 153, 159),	// KeyPercent
				QColor(200, 40, 41),	// Key
				QColor(66, 113, 174),	// Value
				QColor(255, 215, 215),	// ErrorLine
				QColor(240, 240, 240),	// LineNumberBackground
				QColor(160, 160, 158),	// LineNumber
				QColor(77, 77, 76),		// LineNumberCurrent
			};

			return bDark ? Dark : Light;
		}
	}

	VmtHighlighter::VmtHighlighter(QTextDocument *pDocument, bool bDark)
		: QSyntaxHighlighter(pDocument)
		, m_bDark(bDark)
	{
	}

	void VmtHighlighter::setDark(bool bDark)
	{
		if(m_bDark != bDark)
		{
			m_bDark = bDark;
			rehighlight();
		}
	}

	void VmtHighlighter::highlightBlock(const QString &sText)
	{
		const VmtColors::Scheme &Colors = VmtColors::Get(m_bDark);

		bool bQuoted = previousBlockState() == 1;
		bool bKey = true;
		bool bHadCharThisLine = false;
		bool bComment = false;
		QChar LastChar;

		for(int i = 0; i < sText.length(); i++)
		{
			const QChar Character = sText.at(i);
			const QChar NextCharacter = (i + 1) < sText.length() ? sText.at(i + 1) : QChar();

			const bool bValidQuote = Character == QLatin1Char('"') && LastChar != QLatin1Char('\\');
			if(bValidQuote)
			{
				bQuoted = !bQuoted;
			}

			const bool bWhitespace = Character == QLatin1Char(' ') || Character == QLatin1Char('\t');
			const bool bLastWhitespace = LastChar == QLatin1Char(' ') || LastChar == QLatin1Char('\t');

			// The first run of unquoted whitespace after the first token separates key from value.
			if(bWhitespace && !bLastWhitespace && !bQuoted && bHadCharThisLine)
			{
				bKey = false;
			}

			bComment = bComment || (Character == QLatin1Char('/') && NextCharacter == QLatin1Char('/'));

			QColor Color;
			if(bComment)
			{
				Color = Colors.Comment;
			}
			else if(Character == QLatin1Char('{') || Character == QLatin1Char('}') || bValidQuote)
			{
				Color = Colors.Punctuation;
			}
			else if(bKey)
			{
				const bool bAtTokenStart = !bHadCharThisLine || LastChar == QLatin1Char('"');
				if(Character == QLatin1Char('$') && bAtTokenStart)
				{
					Color = Colors.KeyDollar;
				}
				else if(Character == QLatin1Char('%') && bAtTokenStart)
				{
					Color = Colors.KeyPercent;
				}
				else
				{
					Color = Colors.Key;
				}
			}
			else
			{
				Color = (Character == QLatin1Char('[') || Character == QLatin1Char(']'))
					? Colors.Punctuation : Colors.Value;
			}

			setFormat(i, 1, Color);

			bHadCharThisLine = bHadCharThisLine || !bWhitespace;

			LastChar = Character;
		}

		setCurrentBlockState(bQuoted ? 1 : 0);
	}
}
