#include "codeeditor.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextBlock>
#include <QRegExp>
#include <QFont>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    tags_ << "html" << "head" << "title" << "body" << "p" << "a" << "div" << "span"
          << "h1" << "h2" << "h3" << "h4" << "h5" << "h6" << "ul" << "ol" << "li"
          << "b" << "i" << "u" << "em" << "strong" << "br" << "hr" << "img"
          << "table" << "tr" << "td" << "th" << "form" << "input" << "button"
          << "label" << "select" << "option" << "textarea" << "style" << "script"
          << "meta" << "link" << "header" << "footer" << "nav" << "section"
          << "article" << "pre" << "code" << "small";

    QFont f("Courier");
    f.setStyleHint(QFont::TypeWriter);
    f.setFixedPitch(true);
    f.setPixelSize(16);
    setFont(f);

    setLineWrapMode(QPlainTextEdit::WidgetWidth);

    connect(this, SIGNAL(textChanged()), this, SLOT(updateSuggestions()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateSuggestions()));
}

bool CodeEditor::isKnown(const QString &w) const
{
    return w == "!" || tags_.contains(w);
}

bool CodeEditor::isVoid(const QString &t) const
{
    return t == "br" || t == "hr" || t == "img" || t == "input" || t == "meta" || t == "link";
}

QString CodeEditor::expansion(const QString &tag) const
{
    // | oznacza pozycje kursora po rozwinieciu
    if (tag == "!")
        return "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>|</title>\n</head>\n<body>\n\n</body>\n</html>";
    if (tag == "a")      return "<a href=\"|\"></a>";
    if (tag == "img")    return "<img src=\"|\" alt=\"\">";
    if (tag == "link")   return "<link rel=\"stylesheet\" href=\"|\">";
    if (tag == "script") return "<script>|</script>";
    if (tag == "input")  return "<input type=\"text\" name=\"|\">";
    if (tag == "meta")   return "<meta charset=\"utf-8\">|";
    if (isVoid(tag))     return "<" + tag + ">|";
    return "<" + tag + ">|</" + tag + ">";
}

bool CodeEditor::wordBeforeCursor(QString &word, int &start) const
{
    QTextCursor c = textCursor();
    if (c.hasSelection())
        return false;
    int col = c.position() - c.block().position();
    QString left = c.block().text().left(col);
    QRegExp re("^\\s*(!|[a-z][a-z0-9]*)$");
    if (re.indexIn(left) < 0)
        return false;
    word = re.cap(1);
    start = c.position() - word.length();
    return true;
}

void CodeEditor::insertExpansion(int start, int end, const QString &exp)
{
    QString out = exp;
    int caret = out.indexOf('|');
    if (caret >= 0) out.remove(caret, 1);
    else caret = out.length();

    QTextCursor c = textCursor();
    c.beginEditBlock();
    c.setPosition(start);
    c.setPosition(end, QTextCursor::KeepAnchor);
    c.insertText(out);
    c.endEditBlock();
    c.setPosition(start + caret);
    setTextCursor(c);
}

void CodeEditor::autoClose()
{
    QTextCursor c = textCursor();
    int col = c.position() - c.block().position();
    QString blockText = c.block().text();
    QString left = blockText.left(col);

    QRegExp re("<([A-Za-z][A-Za-z0-9]*)(\\s[^<>]*)?>$");
    if (re.indexIn(left) < 0)
        return;
    QString tag = re.cap(1).toLower();
    if (isVoid(tag))
        return;
    QString closing = "</" + tag + ">";
    if (blockText.mid(col).startsWith(closing))
        return;

    int pos = c.position();
    c.insertText(closing);
    c.setPosition(pos);
    setTextCursor(c);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();

    if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
        QString w;
        int s = 0;
        if (wordBeforeCursor(w, s) && isKnown(w)) {
            insertExpansion(s, textCursor().position(), expansion(w));
            return;
        }
    }

    if (key == Qt::Key_Tab) {
        insertPlainText("  ");
        return;
    }

    QPlainTextEdit::keyPressEvent(e);

    if (e->text() == ">")
        autoClose();
}

void CodeEditor::updateSuggestions()
{
    QStringList out;
    QString w;
    int s = 0;
    if (wordBeforeCursor(w, s)) {
        if (w == "!") {
            out << "!";
        } else {
            for (int i = 0; i < tags_.size() && out.size() < 4; ++i)
                if (tags_.at(i).startsWith(w))
                    out << tags_.at(i);
        }
    }
    emit suggestionsChanged(out);
}

void CodeEditor::applySuggestion(const QString &tag)
{
    QString w;
    int s = 0;
    if (!wordBeforeCursor(w, s))
        return;
    insertExpansion(s, textCursor().position(), expansion(tag));
    setFocus();
}
