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

public slots:
    void setMaxFilenames(int);

private:
    QWidget *line_number_area_;
    int max_filenames_ = 0;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(FilenamesEditorWidget *editor) : QWidget(editor), editor_(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(editor_->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        editor_->lineNumberAreaPaintEvent(event);
    }

private:
    FilenamesEditorWidget *editor_;
};