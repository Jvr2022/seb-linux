#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QProgressBar;
class QTextEdit;
QT_END_NAMESPACE

class RuntimeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RuntimeWindow(QWidget *parent = nullptr);

    void setStatusText(const QString &text);
    void setVersionText(const QString &version, const QString &build);

private:
    QLabel *infoLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
    QTextEdit *logView_ = nullptr;
};
