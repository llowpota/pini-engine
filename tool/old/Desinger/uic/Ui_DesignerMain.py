# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:\Dev\DevTool\User\Desinger-ui\DesignerMain.ui'
#
# Created: Tue Jul 22 14:25:38 2014
#      by: pyside-uic 0.2.15 running on PySide 1.2.2
#
# WARNING! All changes made in this file will be lost!

from PySide import QtCore, QtGui

class Ui_DesignerMain(object):
    def setupUi(self, DesignerMain):
        DesignerMain.setObjectName("DesignerMain")
        DesignerMain.resize(1296, 822)
        self.centralwidget = QtGui.QWidget(DesignerMain)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayout = QtGui.QVBoxLayout(self.centralwidget)
        self.verticalLayout.setSpacing(3)
        self.verticalLayout.setContentsMargins(3, 3, 3, 3)
        self.verticalLayout.setObjectName("verticalLayout")
        self.ui_TabWidget = QtGui.QTabWidget(self.centralwidget)
        self.ui_TabWidget.setTabsClosable(True)
        self.ui_TabWidget.setMovable(True)
        self.ui_TabWidget.setObjectName("ui_TabWidget")
        self.verticalLayout.addWidget(self.ui_TabWidget)
        DesignerMain.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(DesignerMain)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 1296, 21))
        self.menubar.setObjectName("menubar")
        self.menuFile = QtGui.QMenu(self.menubar)
        self.menuFile.setObjectName("menuFile")
        self.menuEdit = QtGui.QMenu(self.menubar)
        self.menuEdit.setObjectName("menuEdit")
        self.menuView = QtGui.QMenu(self.menubar)
        self.menuView.setObjectName("menuView")
        self.menuRun = QtGui.QMenu(self.menubar)
        self.menuRun.setObjectName("menuRun")
        self.menuTool = QtGui.QMenu(self.menubar)
        self.menuTool.setObjectName("menuTool")
        self.menuHelp = QtGui.QMenu(self.menubar)
        self.menuHelp.setObjectName("menuHelp")
        DesignerMain.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(DesignerMain)
        self.statusbar.setObjectName("statusbar")
        DesignerMain.setStatusBar(self.statusbar)
        self.saveMenuBtn = QtGui.QAction(DesignerMain)
        self.saveMenuBtn.setObjectName("saveMenuBtn")
        self.newMenuBtn = QtGui.QAction(DesignerMain)
        self.newMenuBtn.setObjectName("newMenuBtn")
        self.openMenuBtn = QtGui.QAction(DesignerMain)
        self.openMenuBtn.setObjectName("openMenuBtn")
        self.saveAsMenuBtn = QtGui.QAction(DesignerMain)
        self.saveAsMenuBtn.setObjectName("saveAsMenuBtn")
        self.exitMenuBtn = QtGui.QAction(DesignerMain)
        self.exitMenuBtn.setObjectName("exitMenuBtn")
        self.menuFile.addAction(self.newMenuBtn)
        self.menuFile.addAction(self.openMenuBtn)
        self.menuFile.addAction(self.saveMenuBtn)
        self.menuFile.addAction(self.saveAsMenuBtn)
        self.menuFile.addSeparator()
        self.menuFile.addAction(self.exitMenuBtn)
        self.menubar.addAction(self.menuFile.menuAction())
        self.menubar.addAction(self.menuEdit.menuAction())
        self.menubar.addAction(self.menuView.menuAction())
        self.menubar.addAction(self.menuRun.menuAction())
        self.menubar.addAction(self.menuTool.menuAction())
        self.menubar.addAction(self.menuHelp.menuAction())

        self.retranslateUi(DesignerMain)
        self.ui_TabWidget.setCurrentIndex(-1)
        QtCore.QMetaObject.connectSlotsByName(DesignerMain)

    def retranslateUi(self, DesignerMain):
        DesignerMain.setWindowTitle(QtGui.QApplication.translate("DesignerMain", "Desinger", None, QtGui.QApplication.UnicodeUTF8))
        self.menuFile.setTitle(QtGui.QApplication.translate("DesignerMain", "&File", None, QtGui.QApplication.UnicodeUTF8))
        self.menuEdit.setTitle(QtGui.QApplication.translate("DesignerMain", "&Edit", None, QtGui.QApplication.UnicodeUTF8))
        self.menuView.setTitle(QtGui.QApplication.translate("DesignerMain", "&View", None, QtGui.QApplication.UnicodeUTF8))
        self.menuRun.setTitle(QtGui.QApplication.translate("DesignerMain", "&Run", None, QtGui.QApplication.UnicodeUTF8))
        self.menuTool.setTitle(QtGui.QApplication.translate("DesignerMain", "&Tool", None, QtGui.QApplication.UnicodeUTF8))
        self.menuHelp.setTitle(QtGui.QApplication.translate("DesignerMain", "&Help", None, QtGui.QApplication.UnicodeUTF8))
        self.saveMenuBtn.setText(QtGui.QApplication.translate("DesignerMain", "저장(&S)", None, QtGui.QApplication.UnicodeUTF8))
        self.saveMenuBtn.setShortcut(QtGui.QApplication.translate("DesignerMain", "Ctrl+S", None, QtGui.QApplication.UnicodeUTF8))
        self.newMenuBtn.setText(QtGui.QApplication.translate("DesignerMain", "새로 만들기(&N)", None, QtGui.QApplication.UnicodeUTF8))
        self.newMenuBtn.setShortcut(QtGui.QApplication.translate("DesignerMain", "Ctrl+N", None, QtGui.QApplication.UnicodeUTF8))
        self.openMenuBtn.setText(QtGui.QApplication.translate("DesignerMain", "열기(&O)", None, QtGui.QApplication.UnicodeUTF8))
        self.openMenuBtn.setShortcut(QtGui.QApplication.translate("DesignerMain", "Ctrl+O", None, QtGui.QApplication.UnicodeUTF8))
        self.saveAsMenuBtn.setText(QtGui.QApplication.translate("DesignerMain", "다른 이름으로 저장(&A)", None, QtGui.QApplication.UnicodeUTF8))
        self.exitMenuBtn.setText(QtGui.QApplication.translate("DesignerMain", "끝내기(&X)", None, QtGui.QApplication.UnicodeUTF8))

