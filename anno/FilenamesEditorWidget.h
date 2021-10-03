#pragma once
#include <QPlainTextEdit>

class FilenamesEditorWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    FilenamesEditorWidget(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(FilenamesEditorWidget *editor) : QWidget(editor), editor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        editor->lineNumberAreaPaintEvent(event);
    }

private:
    FilenamesEditorWidget *editor;
};