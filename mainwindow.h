#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

class QLabel;
class QPushButton;
class QToolButton;
class QCloseEvent;
class CodeEditor;
class Highlighter;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void runPage();
    void onSuggestions(const QStringList &tags);
    void onSuggestionClicked();
    void onKeyClicked();
    void updateTitle();

private:
    bool maybeSave();
    bool writeTo(const QString &path);
    QString previewPath() const;
    QString startDir() const;

    CodeEditor *editor;
    Highlighter *highlighter;
    QLabel *titleLabel;
    QLabel *hintLabel;
    QPushButton *sugBtn[4];
    QString currentFile;
};

#endif
