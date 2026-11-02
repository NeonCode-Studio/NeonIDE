#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent),
      tagRe("</?[A-Za-z][A-Za-z0-9]*|/?>"),
      attrRe("\\b[A-Za-z-]+(?==)"),
      strRe("\"[^\"]*\""),
      docRe("<!DOCTYPE[^>]*>", Qt::CaseInsensitive),
      cStart("<!--"),
      cEnd("-->")
{
    tagFmt.setForeground(QColor(86, 156, 214));
    attrFmt.setForeground(QColor(156, 220, 254));
    strFmt.setForeground(QColor(206, 145, 120));
    commentFmt.setForeground(QColor(106, 153, 85));
    docFmt.setForeground(QColor(128, 128, 128));
}

void Highlighter::applyRe(const QString &text, const QRegExp &re, const QTextCharFormat &fmt)
{
    int idx = 0;
    while ((idx = re.indexIn(text, idx)) >= 0) {
        int len = re.matchedLength();
        if (len <= 0) break;
        setFormat(idx, len, fmt);
        idx += len;
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    applyRe(text, tagRe, tagFmt);
    applyRe(text, attrRe, attrFmt);
    applyRe(text, strRe, strFmt);
    applyRe(text, docRe, docFmt);

    // komentarze <!-- ... --> takze wieloliniowe
    setCurrentBlockState(0);
    int start = (previousBlockState() == 1) ? 0 : cStart.indexIn(text);
    while (start >= 0) {
        int end = cEnd.indexIn(text, start);
        int len;
        if (end == -1) {
            setCurrentBlockState(1);
            len = text.length() - start;
        } else {
            len = end - start + cEnd.matchedLength();
        }
        setFormat(start, len, commentFmt);
        if (len <= 0) break;
        start = cStart.indexIn(text, start + len);
    }
}
