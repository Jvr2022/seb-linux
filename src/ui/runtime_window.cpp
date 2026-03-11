#include "runtime_window.h"

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QScreen>
#include <QTextEdit>
#include <QVBoxLayout>

RuntimeWindow::RuntimeWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Safe Exam Browser"));
    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setWindowFlag(Qt::Dialog, true);
    setFixedSize(720, 320);
    setObjectName(QStringLiteral("RuntimeWindow"));
    setStyleSheet(
        "#RuntimeWindow { background:#e9edf2; }"
        "QFrame#Surface { background:#e9edf2; border-top:3px solid #3c78a6; }"
        "QLabel { color:#213f60; }"
        "QProgressBar { background:transparent; border:0; }"
        "QProgressBar::chunk { background:transparent; }"
        "QTextEdit { background:#ffffff; border:1px solid #d7dee7; color:#5e7388; }");

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *surface = new QFrame(this);
    surface->setObjectName(QStringLiteral("Surface"));
    outerLayout->addWidget(surface);

    auto *layout = new QVBoxLayout(surface);
    layout->setContentsMargins(46, 34, 46, 18);
    layout->setSpacing(0);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(24);
    layout->addLayout(topRow);

    auto *copyColumn = new QVBoxLayout();
    copyColumn->setSpacing(10);
    topRow->addLayout(copyColumn, 1);

    auto *titleLabel = new QLabel(QStringLiteral("Safe Exam Browser"), surface);
    titleLabel->setStyleSheet("font-size:34px; font-weight:800; color:#1f4568;");
    copyColumn->addWidget(titleLabel);

    auto *subtitleLabel = new QLabel(
        QStringLiteral("Preparing the secure exam session and browser environment."),
        surface);
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setStyleSheet("font-size:16px; color:#54708b;");
    copyColumn->addWidget(subtitleLabel);

    copyColumn->addStretch(1);

    infoLabel_ = new QLabel(surface);
    infoLabel_->setStyleSheet("font-size:12px; color:#71859a;");
    copyColumn->addWidget(infoLabel_);

    auto *logoLabel = new QLabel(surface);
    logoLabel->setPixmap(QPixmap(QStringLiteral(":/assets/icons/safe-exam-browser.png")).scaled(
        104,
        104,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    topRow->addWidget(logoLabel, 0, Qt::AlignTop);

    layout->addStretch(1);

    auto *progressShell = new QWidget(surface);
    auto *progressLayout = new QGridLayout(progressShell);
    progressLayout->setContentsMargins(0, 0, 0, 0);

    progressBar_ = new QProgressBar(progressShell);
    progressBar_->setRange(0, 0);
    progressBar_->setTextVisible(false);
    progressBar_->setFixedHeight(18);
    progressLayout->addWidget(progressBar_, 0, 0);

    statusLabel_ = new QLabel(QStringLiteral("Starting Safe Exam Browser..."), progressShell);
    statusLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusLabel_->setStyleSheet("font-size:13px; color:#294e72;");
    progressLayout->addWidget(statusLabel_, 0, 0);
    layout->addWidget(progressShell);

    logView_ = new QTextEdit(surface);
    logView_->setReadOnly(true);
    logView_->setVisible(false);
    layout->addWidget(logView_);

    if (const QScreen *screen = QApplication::primaryScreen()) {
        const QRect geometry = screen->availableGeometry();
        move(
            geometry.left() + (geometry.width() - width()) / 2,
            geometry.top() + (geometry.height() - height()) / 2);
    }
}

void RuntimeWindow::setStatusText(const QString &text)
{
    statusLabel_->setText(text);
}

void RuntimeWindow::setVersionText(const QString &version, const QString &build)
{
    infoLabel_->setText(QStringLiteral("Version %1").arg(version));
    Q_UNUSED(build);
}
