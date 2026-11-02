#include "mainwindow.h"
#include "codeeditor.h"
#include "highlighter.h"

#include <QAction>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPolygon>
#include <QPushButton>
#include <QTextStream>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#define PL(s) QString::fromUtf8(s)

static QPixmap playIcon()
{
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(78, 201, 106));
    QPolygon tri;
    tri << QPoint(8, 4) << QPoint(8, 28) << QPoint(27, 16);
    p.drawPolygon(tri);
    return pm;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---- pasek tytulowy z przyciskiem Uruchom (trojkat w prawym rogu) ----
    QWidget *top = new QWidget(central);
    top->setAutoFillBackground(true);
    QPalette tp = top->palette();
    tp.setColor(QPalette::Window, QColor(0, 122, 204));
    top->setPalette(tp);
    QHBoxLayout *topL = new QHBoxLayout(top);
    topL->setContentsMargins(8, 0, 0, 0);
    topL->setSpacing(0);

    titleLabel = new QLabel(top);
    QPalette lp = titleLabel->palette();
    lp.setColor(QPalette::WindowText, Qt::white);
    titleLabel->setPalette(lp);
    topL->addWidget(titleLabel, 1);

    QToolButton *run = new QToolButton(top);
    run->setIcon(QIcon(playIcon()));
    run->setIconSize(QSize(32, 32));
    run->setFixedSize(56, 42);
    run->setFocusPolicy(Qt::NoFocus);
    topL->addWidget(run, 0);
    connect(run, SIGNAL(clicked()), this, SLOT(runPage()));
    root->addWidget(top);

    // ---- pasek podpowiedzi ----
    QWidget *sugBar = new QWidget(central);
    QHBoxLayout *sugL = new QHBoxLayout(sugBar);
    sugL->setContentsMargins(2, 2, 2, 2);
    sugL->setSpacing(3);
    for (int i = 0; i < 4; ++i) {
        sugBtn[i] = new QPushButton(sugBar);
        sugBtn[i]->setFocusPolicy(Qt::NoFocus);
        sugBtn[i]->setMinimumHeight(34);
        sugBtn[i]->hide();
        connect(sugBtn[i], SIGNAL(clicked()), this, SLOT(onSuggestionClicked()));
        sugL->addWidget(sugBtn[i]);
    }
    hintLabel = new QLabel(PL("Wpisz np. p, div, h1, a, ! i naciśnij spację"), sugBar);
    sugL->addWidget(hintLabel, 1);
    sugL->addStretch(1);
    root->addWidget(sugBar);

    // ---- pasek znakow ----
    QWidget *keyBar = new QWidget(central);
    QHBoxLayout *keyL = new QHBoxLayout(keyBar);
    keyL->setContentsMargins(2, 0, 2, 2);
    keyL->setSpacing(2);
    const char *keys[] = { "<", ">", "/", "\"", "=", "!", "#", "&", ";", ":", "(", ")", "Tab" };
    for (int i = 0; i < 13; ++i) {
        QString k = QString::fromLatin1(keys[i]);
        QPushButton *b = new QPushButton(k == "&" ? QString("&&") : k, keyBar);
        b->setProperty("ins", k == "Tab" ? QString("  ") : k);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumWidth(0);
        b->setMinimumHeight(32);
        connect(b, SIGNAL(clicked()), this, SLOT(onKeyClicked()));
        keyL->addWidget(b);
    }
    root->addWidget(keyBar);

    // ---- edytor ----
    editor = new CodeEditor(central);
    QPalette ep = editor->palette();
    ep.setColor(QPalette::Base, QColor(30, 30, 30));
    ep.setColor(QPalette::Text, QColor(212, 212, 212));
    editor->setPalette(ep);
    highlighter = new Highlighter(editor->document());
    root->addWidget(editor, 1);

    setCentralWidget(central);

    connect(editor, SIGNAL(suggestionsChanged(QStringList)),
            this, SLOT(onSuggestions(QStringList)));
    connect(editor->document(), SIGNAL(modificationChanged(bool)),
            this, SLOT(updateTitle()));

    // ---- menu (Opcje) i klawisze programowe ----
    QMenu *m = menuBar()->addMenu(PL("Plik"));
    QAction *aNew = m->addAction(PL("Nowy"), this, SLOT(newFile()));
    QAction *aOpen = m->addAction(PL("Otwórz..."), this, SLOT(openFile()));
    QAction *aSave = m->addAction(PL("Zapisz"), this, SLOT(saveFile()));
    QAction *aSaveAs = m->addAction(PL("Zapisz jako..."), this, SLOT(saveFileAs()));
    QAction *aRun = m->addAction(PL("Uruchom w przeglądarce"), this, SLOT(runPage()));
    Q_UNUSED(aNew); Q_UNUSED(aOpen); Q_UNUSED(aSave); Q_UNUSED(aSaveAs); Q_UNUSED(aRun);

    QAction *aExit = new QAction(PL("Wyjdź"), this);
    aExit->setSoftKeyRole(QAction::NegativeSoftKey);
    connect(aExit, SIGNAL(triggered()), this, SLOT(close()));
    addAction(aExit);

    editor->setPlainText(PL("<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Test</title>\n</head>\n<body>\n\n</body>\n</html>"));
    editor->document()->setModified(false);
    updateTitle();
    onSuggestions(QStringList());
}

QString MainWindow::previewPath() const
{
#ifdef Q_OS_SYMBIAN
    // publiczny folder, do ktorego ma dostep przegladarka
    return QString("C:/Data/neonide_preview.html");
#else
    return QDir::temp().filePath("neonide_preview.html");
#endif
}

QString MainWindow::startDir() const
{
#ifdef Q_OS_SYMBIAN
    return QString("C:/Data/");
#else
    return QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
}

bool MainWindow::writeTo(const QString &path)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    ts << editor->toPlainText();
    ts.flush();
    f.close();
    return true;
}

void MainWindow::runPage()
{
    QString path = previewPath();
    if (!writeTo(path)) {
        QMessageBox::warning(this, PL("Błąd"), PL("Nie można zapisać pliku podglądu."));
        return;
    }
    // otwiera plik w domyslnej przegladarce telefonu
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
        QMessageBox::warning(this, PL("Błąd"), PL("Nie udało się otworzyć przeglądarki."));
}

void MainWindow::updateTitle()
{
    QString name = currentFile.isEmpty() ? PL("bez nazwy.html") : QFileInfo(currentFile).fileName();
    if (editor->document()->isModified())
        name += "*";
    titleLabel->setText(name);
}

bool MainWindow::maybeSave()
{
    if (!editor->document()->isModified())
        return true;
    QMessageBox::StandardButton r = QMessageBox::question(
        this, PL("neonIDE"), PL("Zapisać zmiany?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (r == QMessageBox::Save)
        return saveFile();
    return r == QMessageBox::Discard;
}

void MainWindow::newFile()
{
    if (!maybeSave())
        return;
    editor->clear();
    currentFile.clear();
    editor->document()->setModified(false);
    updateTitle();
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;
    QString path = QFileDialog::getOpenFileName(this, PL("Otwórz"), startDir(),
                                                "HTML (*.html *.htm);;*");
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, PL("Błąd"), PL("Nie można otworzyć pliku."));
        return;
    }
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    editor->setPlainText(ts.readAll());
    f.close();
    currentFile = path;
    editor->document()->setModified(false);
    updateTitle();
}

bool MainWindow::saveFile()
{
    if (currentFile.isEmpty())
        return saveFileAs();
    if (!writeTo(currentFile)) {
        QMessageBox::warning(this, PL("Błąd"), PL("Nie można zapisać pliku."));
        return false;
    }
    editor->document()->setModified(false);
    updateTitle();
    return true;
}

bool MainWindow::saveFileAs()
{
    QString path = QFileDialog::getSaveFileName(this, PL("Zapisz jako"),
                                                startDir() + "strona.html",
                                                "HTML (*.html *.htm)");
    if (path.isEmpty())
        return false;
    if (!path.endsWith(".html", Qt::CaseInsensitive) && !path.endsWith(".htm", Qt::CaseInsensitive))
        path += ".html";
    currentFile = path;
    return saveFile();
}

void MainWindow::onSuggestions(const QStringList &tags)
{
    for (int i = 0; i < 4; ++i) {
        if (i < tags.size()) {
            QString t = tags.at(i);
            sugBtn[i]->setText(t == "!" ? PL("! szkielet") : "<" + t + ">");
            sugBtn[i]->setProperty("tag", t);
            sugBtn[i]->show();
        } else {
            sugBtn[i]->hide();
        }
    }
    hintLabel->setVisible(tags.isEmpty());
}

void MainWindow::onSuggestionClicked()
{
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    if (b)
        editor->applySuggestion(b->property("tag").toString());
}

void MainWindow::onKeyClicked()
{
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    if (b) {
        editor->insertPlainText(b->property("ins").toString());
        editor->setFocus();
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}
