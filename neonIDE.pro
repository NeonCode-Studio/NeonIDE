QT       += core gui
TARGET    = neonIDE
TEMPLATE  = app
VERSION   = 1.0.0

SOURCES  += main.cpp mainwindow.cpp codeeditor.cpp highlighter.cpp
HEADERS  += mainwindow.h codeeditor.h highlighter.h

symbian {
    # UID z zakresu 0xE0000000-0xEFFFFFFF = do self-signingu (testowy)
    TARGET.UID3 = 0xE6C0DE01
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE  = 0x020000 0x4000000
    DEPLOYMENT.display_name = neonIDE
    ICON = neonIDE.svg
}
