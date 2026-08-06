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

	VmtHighlighter::VmtHighlighter(QTextDocument *pDocument)
		: QSyntaxHighlighter(pDocument)
	{
	}

	void VmtHighlighter::highlightBlock(const QString &sText)
	{
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

			// Unquoted whitespace after the first token separates key from value.
			if((Character == QLatin1Char(' ') || Character == QLatin1Char('\t')) && !bQuoted && bHadCharThisLine)
			{
				bKey = !bKey;
			}

			bComment = bComment || (Character == QLatin1Char('/') && NextCharacter == QLatin1Char('/'));

			QColor Color;
			if(bComment)
			{
				Color = VmtColors::Comment;
			}
			else if(Character == QLatin1Char('{') || Character == QLatin1Char('}') || bValidQuote)
			{
				Color = VmtColors::Punctuation;
			}
			else if(bKey)
			{
				const bool bAtTokenStart = !bHadCharThisLine || LastChar == QLatin1Char('"');
				if(Character == QLatin1Char('$') && bAtTokenStart)
				{
					Color = VmtColors::KeyDollar;
				}
				else if(Character == QLatin1Char('%') && bAtTokenStart)
				{
					Color = VmtColors::KeyPercent;
				}
				else
				{
					Color = VmtColors::Key;
				}
			}
			else
			{
				Color = (Character == QLatin1Char('[') || Character == QLatin1Char(']'))
					? VmtColors::Punctuation : VmtColors::Value;
			}

			setFormat(i, 1, Color);

			bHadCharThisLine = bHadCharThisLine
				|| !(Character == QLatin1Char(' ') || Character == QLatin1Char('\t'));

			LastChar = Character;
		}

		setCurrentBlockState(bQuoted ? 1 : 0);
	}
}
