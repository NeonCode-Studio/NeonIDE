#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit Highlighter(QTextDocument *parent);

protected:
    void highlightBlock(const QString &text);

private:
    void applyRe(const QString &text, const QRegExp &re, const QTextCharFormat &fmt);

    QTextCharFormat tagFmt, attrFmt, strFmt, commentFmt, docFmt;
    QRegExp tagRe, attrRe, strRe, docRe, cStart, cEnd;
};

#endif
