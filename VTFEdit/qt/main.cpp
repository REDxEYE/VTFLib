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

#include "MainWindow.h"
#include "VTFLibQt.h"

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QApplication Application(argc, argv);

	QApplication::setOrganizationName(QStringLiteral("Breadworks"));
	QApplication::setApplicationName(QStringLiteral("VTFEdit++"));
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/app.ico")));

	if(vlGetVersion() != VL_VERSION)
	{
		QMessageBox::critical(nullptr, QApplication::applicationName(),
			QObject::tr("Invalid VTFLib++.dll version number."));
		return 1;
	}

	// Initialize DevIL.
	ILuint uiImage = 0;

	ilInit();

	ilEnable(IL_FILE_OVERWRITE);

	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	ilGenImages(1, &uiImage);
	ilBindImage(uiImage);

	int iResult = 0;
	{
		VTFEdit::MainWindow Window;
		Window.show();

		const QStringList Arguments = QApplication::arguments();
		if(Arguments.count() >= 2)
		{
			Window.openCommandLineFiles(Arguments.mid(1));
		}

		iResult = QApplication::exec();
	}

	// Shutdown DevIL.
	ilDeleteImages(1, &uiImage);
	ilShutDown();

	return iResult;
}
