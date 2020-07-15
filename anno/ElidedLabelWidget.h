#pragma once
#include <QWidget>
#include <QString>

class ElidedLabelWidget : public QWidget
{
    Q_OBJECT       

public:
    ElidedLabelWidget(QWidget *parent = nullptr);

    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(int widthHint READ widthHint WRITE setWidthHint)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    
    void setText(const QString &text);
    const QString & text() const { return content_; }

    void setWidthHint(int value);
    int widthHint() const { return widthHint_; }

    void setElideMode(Qt::TextElideMode value);
    Qt::TextElideMode elideMode() const { return elideMode_; }

    QSize sizeHint() const override { return QSize(widthHint_, 1); }
    
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString content_;
    int widthHint_ = 0;
    Qt::TextElideMode elideMode_ = Qt::ElideLeft;
};