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

#include "BatchConvertSettings.h"
#include "VmtEditorSettings.h"
#include "VtfOptions.h"

#include <QMainWindow>
#include <QPoint>
#include <QStringList>

#include <vector>

class QAction;
class QActionGroup;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QSlider;
class QSpinBox;
class QSplitter;
class QStackedWidget;
class QTabWidget;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

namespace VTFEdit
{
	class AboutDialog;
	class BatchConvertDialog;
	class ImageView;
	class VmtCreateDialog;
	class VmtEditorOptionsDialog;
	class VmtHighlighter;
	class VtfOptionsDialog;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		MainWindow();
		~MainWindow() override;

		void openCommandLineFile(const QString &sFilePath);

	protected:
		void closeEvent(QCloseEvent *pEvent) override;
		void dragEnterEvent(QDragEnterEvent *pEvent) override;
		void dropEvent(QDropEvent *pEvent) override;
		bool eventFilter(QObject *pObject, QEvent *pEvent) override;

	private slots:
		void onNew();
		void onOpen();
		void onSave();
		void onSaveAs();
		void onImport();
		void onExport();
		void onExportAll();
		void onCreateVmtFile();
		void onConvertFolder();
		void onRecentFile();
		void onCopy();
		void onPaste();
		void onChannelChanged();
		void onViewOptionChanged();
		void onAbout();
		void onClipboardChanged();

		void onImageParameterChanged();
		void onAnimateClicked();
		void onAnimateFpsChanged(int iFps);
		void onAnimateTick();
		void onFlagItemChanged(QListWidgetItem *pItem);
		void onHdrReset();

		void onZoomIn();
		void onZoomOut();
		void onZoomReset();

		void onVmtTextChanged();
		void onVmtCursorChanged();
		void onValidateLoose();
		void onValidateStrict();
		void onVmtEditorOptions();

		void onImageContextMenu(const QPoint &Position);
		void onVmtContextMenu(const QPoint &Position);
		void onImageMouseMoved(int iX, int iY);

	private:
		// Construction.
		void createActions();
		void createMenus();
		void createToolBar();
		void createStatusBar();
		void createCentralWidget();
		QWidget *createImageTab();
		QWidget *createInfoTab();
		QWidget *createResourcesTab();

		// File operations.
		void newFile();
		void open(const QString &sFileName, bool bTemp);
		void save(const QString &sFileName);
		void saveAs();
		void import(const QStringList &sFileNames);
		void createFromImages(const std::vector<vlByte *> &vImageData, vlUInt uiWidth, vlUInt uiHeight,
			bool bHasAlpha);
		void exportImage(const QString &sFileName);
		void exportAllImages(const QString &sFileName);
		void closeFile();

		// VTF/VMT plumbing.
		void updateVtfFile();
		void setVtfFile(VTFLib::CVTFFile *pVTFFile);
		bool getVtfFile();
		void setVmtFile(VTFLib::CVMTFile *pVMTFile);
		bool validateVmtFile();
		bool confirmVmtFile();
		void setResourceInformation(QTreeWidgetItem *pItem, VTFLib::Nodes::CVMTGroupNode *pVMTNode);
		void updateVmtErrorHighlight();
		void applyVmtEditorSettings();

		// View.
		void zoomAt(float fFactor, const QPoint &Anchor);
		void zoom(float fFactor);
		void updateSidebarsVisible();

		// Recent files and configuration.
		void addRecentFile(const QString &sFileName);
		void updateRecentFiles();
		QString configFilePath() const;
		bool readConfigFile(const QString &sConfigFile);
		bool writeConfigFile(const QString &sConfigFile) const;

		void setFileName(const QString &sFileName);
		void handleDroppedFiles(const QStringList &sFiles);

		VTFLib::CVMTFile *m_pVMTFile;
		VTFLib::CVTFFile *m_pVTFFile;

		QString m_sFileName;

		float m_fImageScale;
		float m_fEffectiveImageScale;

		bool m_bImagePanning;
		QPoint m_ImagePanStartMouse;
		QPoint m_ImagePanStartScroll;

		bool m_bUpdatingVtfFile;
		bool m_bUpdatingFlags;
		bool m_bHdrResetting;

		int m_iMaximumRecentFiles;
		QStringList m_RecentFiles;

		VtfOptions m_Options;
		BatchConvertSettings m_BatchConvertSettings;
		VmtEditorSettings m_VmtEditorSettings;
		VtfOptionsDialog *m_pOptionsDialog;
		VmtCreateDialog *m_pVmtCreateDialog;
		VmtEditorOptionsDialog *m_pVmtEditorOptionsDialog;
		BatchConvertDialog *m_pBatchConvertDialog;
		AboutDialog *m_pAboutDialog;

		// Actions.
		QAction *m_pNewAction;
		QAction *m_pOpenAction;
		QAction *m_pSaveAction;
		QAction *m_pSaveAsAction;
		QAction *m_pImportAction;
		QAction *m_pExportAction;
		QAction *m_pExportAllAction;
		QAction *m_pExitAction;
		QAction *m_pCopyAction;
		QAction *m_pPasteAction;
		QAction *m_pChannelRgbAction;
		QAction *m_pChannelRAction;
		QAction *m_pChannelGAction;
		QAction *m_pChannelBAction;
		QAction *m_pChannelAAction;
		QActionGroup *m_pChannelGroup;
		QAction *m_pMaskAction;
		QAction *m_pTileAction;
		QAction *m_pMipmapFullSizeAction;
		QAction *m_pCreateVmtFileAction;
		QAction *m_pConvertFolderAction;
		QAction *m_pAutoCreateVmtFileAction;
		QAction *m_pVmtEditorOptionsAction;
		QAction *m_pAboutAction;
		QMenu *m_pRecentFilesMenu;

		// Image view.
		QStackedWidget *m_pCentralStack;
		QScrollArea *m_pImageScrollArea;
		ImageView *m_pImageView;
		QMenu *m_pImageContextMenu;

		// VMT editor.
		QPlainTextEdit *m_pVmtEdit;
		VmtHighlighter *m_pVmtHighlighter;
		int m_iVmtErrorLine;

		// Sidebars.
		QSplitter *m_pSplitter;
		QTabWidget *m_pLeftTabs;
		QTabWidget *m_pRightTabs;
		QWidget *m_pImageTab;
		QWidget *m_pInfoTab;
		QWidget *m_pResourcesTab;

		// Image tab controls.
		QSpinBox *m_pFrame;
		QSpinBox *m_pFace;
		QSpinBox *m_pSlice;
		QSpinBox *m_pMipmap;
		QSlider *m_pHdrExposure;
		QPushButton *m_pAnimateButton;
		QSpinBox *m_pAnimateFps;
		QTimer *m_pAnimateTimer;
		QListWidget *m_pFlags;

		// Info tab controls.
		QLabel *m_pFileVersion;
		QLabel *m_pFileSize;
		QLabel *m_pFileCompression;
		QLabel *m_pImageWidth;
		QLabel *m_pImageHeight;
		QLabel *m_pImageFormat;
		QLabel *m_pImageFrames;
		QSpinBox *m_pImageStartFrame;
		QLabel *m_pImageFaces;
		QLabel *m_pImageSlices;
		QLabel *m_pImageMipmaps;
		QDoubleSpinBox *m_pImageBumpmapScale;
		QLabel *m_pImageReflectivity;
		QLabel *m_pThumbnailWidth;
		QLabel *m_pThumbnailHeight;
		QLabel *m_pThumbnailFormat;

		// Resources tab controls.
		QLabel *m_pResourceCount;
		QTreeWidget *m_pResources;

		// Status bar.
		QLabel *m_pStatusFileName;
		QLabel *m_pStatusInfo1;
		QLabel *m_pStatusInfo2;
	};
}
