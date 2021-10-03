#include "FilenamesEditorWidget.h"

FilenamesEditorWidget::FilenamesEditorWidget(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &FilenamesEditorWidget::blockCountChanged, this, &FilenamesEditorWidget::updateLineNumberAreaWidth);
    connect(this, &FilenamesEditorWidget::updateRequest, this, &FilenamesEditorWidget::updateLineNumberArea);
    connect(this, &FilenamesEditorWidget::cursorPositionChanged, this, &FilenamesEditorWidget::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int FilenamesEditorWidget::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void FilenamesEditorWidget::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void FilenamesEditorWidget::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void FilenamesEditorWidget::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void FilenamesEditorWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void FilenamesEditorWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    // Count how many non-empty strings we have so far
    int stringNumber = 0;
    auto previous = block.previous();
    for (; previous.isValid(); previous = previous.previous()) {
        if (!previous.text().trimmed().isEmpty()) {
            ++stringNumber;
        }
    }

    // Draw the rest of blocks
    while (block.isValid() && top <= event->rect().bottom()) {
        auto blockEmpty = block.text().trimmed().isEmpty();
        if (!blockEmpty) {
            ++stringNumber;
        }
        
        if (block.isVisible() && bottom >= event->rect().top()) {
            if (!blockEmpty) {
                QString number = QString::number(stringNumber);
                painter.setPen(Qt::black);
                painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                    Qt::AlignRight, number);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

