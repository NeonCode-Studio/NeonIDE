#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QStringList>

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = 0);

public slots:
    void applySuggestion(const QString &tag);

signals:
    void suggestionsChanged(const QStringList &tags);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void updateSuggestions();

private:
    bool wordBeforeCursor(QString &word, int &start) const;
    QString expansion(const QString &tag) const;
    bool isKnown(const QString &word) const;
    bool isVoid(const QString &tag) const;
    void insertExpansion(int start, int end, const QString &exp);
    void autoClose();

    QStringList tags_;
};

#endif
