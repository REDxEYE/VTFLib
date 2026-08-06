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

#include "ImageView.h"

#include <QMouseEvent>
#include <QPainter>

namespace VTFEdit
{
	ImageView::ImageView(QWidget *pParent)
		: QWidget(pParent)
		, m_bTiled(false)
	{
		setMouseTracking(true);
		setContextMenuPolicy(Qt::CustomContextMenu);
	}

	void ImageView::setImage(const QImage &Image)
	{
		m_Image = Image;
		updateGeometryForImage();
		update();
	}

	void ImageView::setTiled(bool bTiled)
	{
		if(m_bTiled == bTiled)
		{
			return;
		}

		m_bTiled = bTiled;
		updateGeometryForImage();
		update();
	}

	void ImageView::updateGeometryForImage()
	{
		const int iTiles = m_bTiled ? 2 : 1;
		const QSize Size = m_Image.isNull()
			? QSize(0, 0)
			: QSize(m_Image.width() * iTiles, m_Image.height() * iTiles);

		setFixedSize(Size);
	}

	void ImageView::paintEvent(QPaintEvent *pEvent)
	{
		if(m_Image.isNull())
		{
			return;
		}

		QPainter Painter(this);

		const int iTiles = m_bTiled ? 2 : 1;
		for(int j = 0; j < iTiles; j++)
		{
			for(int i = 0; i < iTiles; i++)
			{
				const QRect Target(i * m_Image.width(), j * m_Image.height(),
					m_Image.width(), m_Image.height());

				if(Target.intersects(pEvent->rect()))
				{
					Painter.drawImage(Target.topLeft(), m_Image);
				}
			}
		}
	}

	void ImageView::mouseMoveEvent(QMouseEvent *pEvent)
	{
		if(!m_Image.isNull())
		{
			const QPoint Position = pEvent->position().toPoint();

			emit mouseMovedOverImage(Position.x() % m_Image.width(), Position.y() % m_Image.height());
		}

		QWidget::mouseMoveEvent(pEvent);
	}
}
