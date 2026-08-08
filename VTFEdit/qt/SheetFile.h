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

#include <QByteArray>
#include <QVector>

namespace VTFEdit
{
	// Layout of sprite sheet resource:
	//
	//   int32   version                  (0 or 1)
	//   int32   sequence count
	//   for each sequence:
	//     int32 sequence number          (0 .. SheetMaxSequences - 1)
	//     int32 clamp                    (non zero clamps, zero loops)
	//     int32 frame count
	//     float total sequence duration
	//     for each frame:
	//       float duration
	//       for each image (1 in version 0, SheetImagesPerFrame in version 1):
	//         float left, top, right, bottom  (normalized texture coordinates)
	//
	// Texture coordinates are pixel centres, so a frame covering pixels 
	//   [x, x + w) 
	// maps to
	//   [(x + 0.5) / width, (x + w - 0.5) / width]

	enum
	{
		SheetMaxSequences = 64,
		SheetImagesPerFrame = 4,
		SheetVersion = 1,
	};

	struct SheetCoords
	{
		float fLeft;
		float fTop;
		float fRight;
		float fBottom;

		SheetCoords()
			: fLeft(0.0f)
			, fTop(0.0f)
			, fRight(1.0f)
			, fBottom(1.0f)
		{
		}
	};

	struct SheetFrame
	{
		float fDuration;
		SheetCoords Images[SheetImagesPerFrame];

		SheetFrame()
			: fDuration(1.0f)
		{
		}
	};

	struct SheetSequence
	{
		int iNumber;
		bool bClamp;
		QVector<SheetFrame> Frames;

		SheetSequence()
			: iNumber(0)
			, bClamp(true)
		{
		}
	};

	class SheetFile
	{
	public:
		SheetFile();

		bool load(const void *lpData, unsigned int uiSize);
		QByteArray save() const;

		bool isEmpty() const { return m_Sequences.isEmpty(); }

		// the lowest sequence number not already in use, or -1 if the sheet is full
		int nextFreeSequenceNumber() const;

		QVector<SheetSequence> &sequences() { return m_Sequences; }
		const QVector<SheetSequence> &sequences() const { return m_Sequences; }

		static SheetCoords rectToCoords(int iX, int iY, int iWidth, int iHeight,
			int iImageWidth, int iImageHeight);
		static void coordsToRect(const SheetCoords &Coords, int iImageWidth, int iImageHeight,
			int &iX, int &iY, int &iWidth, int &iHeight);

	private:
		QVector<SheetSequence> m_Sequences;
	};
}
