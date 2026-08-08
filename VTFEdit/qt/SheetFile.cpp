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

#include "SheetFile.h"

#include <QDataStream>
#include <QIODevice>
#include <QtGlobal>

#include <cmath>

namespace VTFEdit
{
	SheetFile::SheetFile()
	{
	}

	bool SheetFile::load(const void *lpData, unsigned int uiSize)
	{
		m_Sequences.clear();

		if(lpData == nullptr || uiSize < 2 * sizeof(qint32))
		{
			return false;
		}

		QByteArray Data = QByteArray::fromRawData(static_cast<const char *>(lpData),
			static_cast<qsizetype>(uiSize));
		QDataStream Stream(Data);
		Stream.setByteOrder(QDataStream::LittleEndian);
		Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

		qint32 iVersion = 0;
		qint32 iSequenceCount = 0;
		Stream >> iVersion >> iSequenceCount;

		if(iVersion != 0 && iVersion != 1)
		{
			return false;
		}

		if(iSequenceCount < 0 || iSequenceCount > SheetMaxSequences)
		{
			return false;
		}

		const int iCoordsPerFrame = iVersion == 0 ? 1 : SheetImagesPerFrame;

		for(int i = 0; i < iSequenceCount; i++)
		{
			qint32 iNumber = 0;
			qint32 iClamp = 0;
			qint32 iFrameCount = 0;
			float fTotalDuration = 0.0f;
			Stream >> iNumber 
				   >> iClamp 
				   >> iFrameCount 
				   >> fTotalDuration;

			if(Stream.status() != QDataStream::Ok)
			{
				m_Sequences.clear();
				return false;
			}

			if(iNumber < 0 || iNumber >= SheetMaxSequences
				|| iFrameCount < 0)
			{
				m_Sequences.clear();
				return false;
			}

			SheetSequence Sequence;
			Sequence.iNumber = static_cast<int>(iNumber);
			Sequence.bClamp = iClamp != 0;
			Sequence.Frames.reserve(static_cast<qsizetype>(iFrameCount));

			for(int j = 0; j < iFrameCount; j++)
			{
				SheetFrame Frame;
				Stream >> Frame.fDuration;

				for(int k = 0; k < iCoordsPerFrame; k++)
				{
					SheetCoords &Coords = Frame.Images[k];
					Stream >> Coords.fLeft 
						   >> Coords.fTop
						   >> Coords.fRight
						   >> Coords.fBottom;
				}

				for(int k = iCoordsPerFrame; k < SheetImagesPerFrame; k++)
				{
					Frame.Images[k] = Frame.Images[0];
				}

				if(Stream.status() != QDataStream::Ok)
				{
					m_Sequences.clear();
					return false;
				}

				Sequence.Frames.append(Frame);
			}

			m_Sequences.append(Sequence);
		}

		return true;
	}

	QByteArray SheetFile::save() const
	{
		QByteArray Data;
		QDataStream Stream(&Data, QIODevice::WriteOnly);
		Stream.setByteOrder(QDataStream::LittleEndian);
		Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

		Stream << static_cast<qint32>(SheetVersion)
			   << static_cast<qint32>(m_Sequences.count());

		for(const SheetSequence &Sequence : m_Sequences)
		{
			float fTotalDuration = 0.0f;
			for(const SheetFrame &Frame : Sequence.Frames)
			{
				fTotalDuration += Frame.fDuration;
			}

			Stream << static_cast<qint32>(Sequence.iNumber)
				   << static_cast<qint32>(Sequence.bClamp ? 1 : 0)
				   << static_cast<qint32>(Sequence.Frames.count())
				   << fTotalDuration;

			for(const SheetFrame &Frame : Sequence.Frames)
			{
				Stream << Frame.fDuration;

				for(int i = 0; i < SheetImagesPerFrame; i++)
				{
					const SheetCoords &Coords = Frame.Images[i];
					Stream << Coords.fLeft
						   << Coords.fTop 
						   << Coords.fRight 
						   << Coords.fBottom;
				}
			}
		}

		return Data;
	}

	int SheetFile::nextFreeSequenceNumber() const
	{
		for(int i = 0; i < SheetMaxSequences; i++)
		{
			bool bUsed = false;
			for(const SheetSequence &Sequence : m_Sequences)
			{
				if(Sequence.iNumber == i)
				{
					bUsed = true;
					break;
				}
			}

			if(!bUsed)
			{
				return i;
			}
		}

		return -1;
	}

	SheetCoords SheetFile::rectToCoords(int iX, int iY, int iWidth, int iHeight,
		int iImageWidth, int iImageHeight)
	{
		SheetCoords Coords;

		if(iImageWidth <= 0 || iImageHeight <= 0)
		{
			return Coords;
		}

		const float fWidth = static_cast<float>(iImageWidth);
		const float fHeight = static_cast<float>(iImageHeight);

		Coords.fLeft = (static_cast<float>(iX) + 0.5f) / fWidth;
		Coords.fTop = (static_cast<float>(iY) + 0.5f) / fHeight;
		Coords.fRight = (static_cast<float>(iX + iWidth - 1) + 0.5f) / fWidth;
		Coords.fBottom = (static_cast<float>(iY + iHeight - 1) + 0.5f) / fHeight;

		return Coords;
	}

	void SheetFile::coordsToRect(const SheetCoords &Coords, int iImageWidth, int iImageHeight,
		int &iX, int &iY, int &iWidth, int &iHeight)
	{
		iX = 0;
		iY = 0;
		iWidth = iImageWidth;
		iHeight = iImageHeight;

		if(iImageWidth <= 0 || iImageHeight <= 0)
		{
			return;
		}

		const float fWidth = static_cast<float>(iImageWidth);
		const float fHeight = static_cast<float>(iImageHeight);

		const int iLeft = static_cast<int>(std::floor(Coords.fLeft * fWidth));
		const int iTop = static_cast<int>(std::floor(Coords.fTop * fHeight));
		const int iRight = static_cast<int>(std::floor(Coords.fRight * fWidth));
		const int iBottom = static_cast<int>(std::floor(Coords.fBottom * fHeight));

		iX = qBound(0, iLeft, iImageWidth - 1);
		iY = qBound(0, iTop, iImageHeight - 1);
		iWidth = qBound(1, iRight - iX + 1, iImageWidth - iX);
		iHeight = qBound(1, iBottom - iY + 1, iImageHeight - iY);
	}
}
