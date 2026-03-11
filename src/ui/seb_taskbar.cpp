#include "seb_taskbar.h"

#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QTimer>

namespace {

QPushButton *createButton(const QString &text, QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setMinimumHeight(30);
    button->setFlat(true);
    QPalette palette = button->palette();
    palette.setColor(QPalette::ButtonText, Qt::black);
    button->setPalette(palette);
    return button;
}

QFrame *createDivider(QWidget *parent)
{
    auto *divider = new QFrame(parent);
    divider->setFrameShape(QFrame::VLine);
    divider->setFrameShadow(QFrame::Plain);
    divider->setLineWidth(1);
    divider->setMidLineWidth(0);
    divider->setStyleSheet("color: #b8b8b8;");
    return divider;
}

}  // namespace

SebTaskbar::SebTaskbar(const seb::SebSettings &settings, bool isMainWindow, QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(54);
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(235, 235, 235));
    palette.setColor(QPalette::WindowText, Qt::black);
    setPalette(palette);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    applicationArea_ = new QWidget(this);
    applicationLayout_ = new QHBoxLayout(applicationArea_);
    applicationLayout_->setContentsMargins(0, 0, 0, 0);
    applicationLayout_->setSpacing(6);
    layout->addWidget(applicationArea_, 0);

    if (isMainWindow) {
        auto *homeButton = createButton(QStringLiteral("Home"), applicationArea_);
        homeButton->setEnabled(settings.browser.mainWindow.showHomeButton && !settings.browser.homeUrl.isEmpty());
        connect(homeButton, &QPushButton::clicked, this, &SebTaskbar::homeRequested);
        applicationLayout_->addWidget(homeButton);
    }

    auto *backButton = createButton(QStringLiteral("Back"), applicationArea_);
    backButton->setEnabled(settings.browser.mainWindow.allowBackwardNavigation);
    connect(backButton, &QPushButton::clicked, this, &SebTaskbar::backRequested);
    applicationLayout_->addWidget(backButton);

    auto *forwardButton = createButton(QStringLiteral("Forward"), applicationArea_);
    forwardButton->setEnabled(settings.browser.mainWindow.allowForwardNavigation);
    connect(forwardButton, &QPushButton::clicked, this, &SebTaskbar::forwardRequested);
    applicationLayout_->addWidget(forwardButton);

    layout->addWidget(createDivider(this));

    metaArea_ = new QWidget(this);
    metaArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    metaLayout_ = new QHBoxLayout(metaArea_);
    metaLayout_->setContentsMargins(0, 0, 0, 0);
    metaLayout_->setSpacing(8);
    layout->addWidget(metaArea_, 1);

    titleLabel_ = new QLabel(QStringLiteral("Safe Exam Browser"), metaArea_);
    metaLayout_->addWidget(titleLabel_);

    statusLabel_ = new QLabel(QStringLiteral("about:blank"), metaArea_);
    statusLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    statusLabel_->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    statusLabel_->setMargin(4);
    QPalette statusPalette = statusLabel_->palette();
    statusPalette.setColor(QPalette::WindowText, Qt::black);
    statusLabel_->setPalette(statusPalette);
    metaLayout_->addWidget(statusLabel_, 1);

    auto *badge = new QLabel(QStringLiteral("EXAM MODE"), metaArea_);
    QPalette badgePalette = badge->palette();
    badgePalette.setColor(QPalette::WindowText, Qt::black);
    badge->setPalette(badgePalette);
    metaLayout_->addWidget(badge);

    auto *reloadButton = createButton(QStringLiteral("Reload"), metaArea_);
    reloadButton->setEnabled(settings.browser.mainWindow.allowReloading);
    connect(reloadButton, &QPushButton::clicked, this, &SebTaskbar::reloadRequested);
    metaLayout_->addWidget(reloadButton);

    auto *findButton = createButton(QStringLiteral("Find"), metaArea_);
    findButton->setEnabled(settings.browser.allowFind);
    connect(findButton, &QPushButton::clicked, this, &SebTaskbar::findRequested);
    metaLayout_->addWidget(findButton);

    layout->addWidget(createDivider(this));

    clockLabel_ = new QLabel(QStringLiteral("--:--"), this);
    clockLabel_->setAlignment(Qt::AlignCenter);
    clockLabel_->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    clockLabel_->setMargin(4);
    QPalette clockPalette = clockLabel_->palette();
    clockPalette.setColor(QPalette::WindowText, Qt::black);
    clockLabel_->setPalette(clockPalette);
    layout->addWidget(clockLabel_);

    layout->addWidget(createDivider(this));

    auto *quitButton = createButton(QStringLiteral("Quit"), this);
    connect(quitButton, &QPushButton::clicked, this, &SebTaskbar::quitRequested);
    layout->addWidget(quitButton);

    clockTimer_ = new QTimer(this);
    clockTimer_->setInterval(1000);
    connect(clockTimer_, &QTimer::timeout, this, &SebTaskbar::refreshClock);
    clockTimer_->start();
    refreshClock();
}

void SebTaskbar::setCurrentUrl(const QUrl &url)
{
    const QString text = url.toString(QUrl::FullyDecoded).trimmed();
    statusLabel_->setText(text.isEmpty() ? QStringLiteral("about:blank") : text);
}

void SebTaskbar::setWindowTitleText(const QString &title)
{
    const QString trimmed = title.trimmed();
    if (!trimmed.isEmpty()) {
        titleLabel_->setText(trimmed);
    }
}

void SebTaskbar::refreshClock()
{
    clockLabel_->setText(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm")));
}
