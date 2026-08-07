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

#include "VmtTextEdit.h"

#include <QPainter>
#include <QTextBlock>

namespace VTFEdit
{
	namespace
	{
		class LineNumberArea : public QWidget
		{
		public:
			explicit LineNumberArea(VmtTextEdit *pEditor)
				: QWidget(pEditor)
				, m_pEditor(pEditor)
			{
			}

			QSize sizeHint() const override
			{
				return QSize(m_pEditor->lineNumberAreaWidth(), 0);
			}

		protected:
			void paintEvent(QPaintEvent *pEvent) override
			{
				m_pEditor->lineNumberAreaPaintEvent(pEvent);
			}

		private:
			VmtTextEdit *m_pEditor;
		};

		const int iLineNumberMargin = 6;
	}

	VmtTextEdit::VmtTextEdit(QWidget *pParent)
		: QPlainTextEdit(pParent)
		, m_pLineNumberArea(new LineNumberArea(this))
		, m_LineNumberBackground(palette().color(QPalette::Window))
		, m_LineNumberText(palette().color(QPalette::WindowText))
		, m_LineNumberCurrentText(palette().color(QPalette::WindowText))
	{
		connect(this, &QPlainTextEdit::blockCountChanged, this, &VmtTextEdit::updateLineNumberAreaWidth);
		connect(this, &QPlainTextEdit::updateRequest, this, &VmtTextEdit::updateLineNumberArea);
		connect(this, &QPlainTextEdit::cursorPositionChanged, m_pLineNumberArea,
			QOverload<>::of(&QWidget::update));

		updateLineNumberAreaWidth();
	}

	void VmtTextEdit::setLineNumberColors(const QColor &Background, const QColor &Text, const QColor &CurrentText)
	{
		m_LineNumberBackground = Background;
		m_LineNumberText = Text;
		m_LineNumberCurrentText = CurrentText;

		m_pLineNumberArea->update();
	}

	int VmtTextEdit::lineNumberAreaWidth() const
	{
		int iDigits = 1;
		for(int iMax = qMax(1, blockCount()); iMax >= 10; iMax /= 10)
		{
			iDigits++;
		}

		return 2 * iLineNumberMargin + fontMetrics().horizontalAdvance(QLatin1Char('9')) * iDigits;
	}

	void VmtTextEdit::lineNumberAreaPaintEvent(QPaintEvent *pEvent)
	{
		QPainter Painter(m_pLineNumberArea);
		Painter.fillRect(pEvent->rect(), m_LineNumberBackground);
		Painter.setFont(font());

		const int iCurrent = textCursor().blockNumber();
		const int iWidth = m_pLineNumberArea->width() - iLineNumberMargin;

		QTextBlock Block = firstVisibleBlock();
		int iTop = static_cast<int>(blockBoundingGeometry(Block).translated(contentOffset()).top());

		while(Block.isValid() && iTop <= pEvent->rect().bottom())
		{
			const int iBottom = iTop + static_cast<int>(blockBoundingRect(Block).height());

			if(Block.isVisible() && iBottom >= pEvent->rect().top())
			{
				Painter.setPen(Block.blockNumber() == iCurrent ? m_LineNumberCurrentText : m_LineNumberText);
				Painter.drawText(0, iTop, iWidth, fontMetrics().height(),
					Qt::AlignRight | Qt::AlignVCenter, QString::number(Block.blockNumber() + 1));
			}

			Block = Block.next();
			iTop = iBottom;
		}
	}

	void VmtTextEdit::resizeEvent(QResizeEvent *pEvent)
	{
		QPlainTextEdit::resizeEvent(pEvent);

		const QRect Contents = contentsRect();
		m_pLineNumberArea->setGeometry(
			QRect(Contents.left(), Contents.top(), lineNumberAreaWidth(), Contents.height()));
	}

	void VmtTextEdit::changeEvent(QEvent *pEvent)
	{
		QPlainTextEdit::changeEvent(pEvent);

		if(pEvent->type() == QEvent::FontChange)
		{
			updateLineNumberAreaWidth();
			m_pLineNumberArea->update();
		}
	}

	void VmtTextEdit::updateLineNumberAreaWidth()
	{
		const int iWidth = lineNumberAreaWidth();
		setViewportMargins(iWidth, 0, 0, 0);

		const QRect Contents = contentsRect();
		m_pLineNumberArea->setGeometry(QRect(Contents.left(), Contents.top(), iWidth, Contents.height()));
	}

	void VmtTextEdit::updateLineNumberArea(const QRect &Rect, int iDy)
	{
		if(iDy != 0)
		{
			m_pLineNumberArea->scroll(0, iDy);
		}
		else
		{
			m_pLineNumberArea->update(0, Rect.y(), m_pLineNumberArea->width(), Rect.height());
		}

		if(Rect.contains(viewport()->rect()))
		{
			updateLineNumberAreaWidth();
		}
	}
}
