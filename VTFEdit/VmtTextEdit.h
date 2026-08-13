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

#include <QColor>
#include <QPlainTextEdit>

namespace VTFEdit
{
	class VmtTextEdit : public QPlainTextEdit
	{
		Q_OBJECT

	public:
		explicit VmtTextEdit(QWidget *pParent = nullptr);

		void setLineNumberColors(const QColor &Background, const QColor &Text, const QColor &CurrentText);

		int lineNumberAreaWidth() const;
		void lineNumberAreaPaintEvent(QPaintEvent *pEvent);

	protected:
		void resizeEvent(QResizeEvent *pEvent) override;
		void changeEvent(QEvent *pEvent) override;

	private slots:
		void updateLineNumberAreaWidth();
		void updateLineNumberArea(const QRect &Rect, int iDy);

	private:
		QWidget *m_pLineNumberArea;
		QColor m_LineNumberBackground;
		QColor m_LineNumberText;
		QColor m_LineNumberCurrentText;
	};
}
