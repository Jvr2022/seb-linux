#include "network_control.h"

#include "../common/taskbar_button.h"
#include "../common/taskbar_popup.h"
#include "../../../shell/taskbar/platform/network_controller.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

namespace {

QString iconForNetwork(const seb::shell::taskbar::platform::NetworkState &state)
{
    if (!state.available) return QStringLiteral(":/assets/taskbar/wifi_0.svg");
    if (state.type == seb::shell::taskbar::platform::NetworkState::Type::Wired) return QStringLiteral(":/assets/taskbar/network_wired.svg");
    if (state.status != seb::shell::taskbar::platform::NetworkState::Status::Connected) return QStringLiteral(":/assets/taskbar/wifi_0.svg");

    int strength = 0;
    for (const auto &network : state.networks) {
        if (network.active) {
            strength = network.signalPercent;
            break;
        }
    }
    if (strength > 66) return QStringLiteral(":/assets/taskbar/wifi_100.svg");
    if (strength > 33) return QStringLiteral(":/assets/taskbar/wifi_66.svg");
    if (strength > 0) return QStringLiteral(":/assets/taskbar/wifi_33.svg");
    return QStringLiteral(":/assets/taskbar/wifi_0.svg");
}

}  // namespace

NetworkControl::NetworkControl(seb::shell::taskbar::platform::NetworkController &controller, QWidget *parent)
    : QWidget(parent)
    , controller_(controller)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(40);
    layout->addWidget(button_);

    popup_ = new TaskbarPopup(this);
    popupLayout_ = new QVBoxLayout(popup_);
    popupLayout_->setContentsMargins(0, 0, 0, 0);
    popupLayout_->setSpacing(0);

    connect(button_, &QPushButton::clicked, this, [this] {
        button_->setHasPopupOpen(true);
        rebuildPopup();
        popup_->showAbove(button_);
    });
    connect(&controller_, &seb::shell::taskbar::platform::NetworkController::stateChanged, this, &NetworkControl::updateState);
    connect(popup_, &TaskbarPopup::popupHidden, this, [this] {
        button_->setHasPopupOpen(false);
    });

    updateState();
}

void NetworkControl::rebuildPopup()
{
    QLayoutItem *item = nullptr;
    while ((item = popupLayout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    const auto &state = controller_.state();
    if (!state.available) {
        auto *label = new QLabel(QStringLiteral("No network interface available."), popup_);
        label->setContentsMargins(10, 10, 10, 10);
        popupLayout_->addWidget(label);
        return;
    }
    if (state.type == seb::shell::taskbar::platform::NetworkState::Type::Wireless) {
        for (const auto &network : state.networks) {
            auto *row = new QPushButton(popup_);
            row->setFlat(true);
            row->setCursor(Qt::PointingHandCursor);
            row->setMinimumHeight(40);
            row->setStyleSheet(QStringLiteral("text-align:left; padding:10px 10px; border:none; font-weight:600;"));
            row->setText(QStringLiteral("%1  %2 (%3%)")
                             .arg(network.active ? QStringLiteral("•") : QStringLiteral(" "))
                             .arg(network.ssid)
                             .arg(network.signalPercent));
            connect(row, &QPushButton::clicked, popup_, &QWidget::hide);
            popupLayout_->addWidget(row);
        }
    }
}

void NetworkControl::updateState()
{
    const auto &state = controller_.state();
    QString tooltip = QStringLiteral("Network unavailable");
    if (state.type == seb::shell::taskbar::platform::NetworkState::Type::Wired) {
        tooltip = state.status == seb::shell::taskbar::platform::NetworkState::Status::Connected
            ? QStringLiteral("Wired network connected")
            : QStringLiteral("Wired network disconnected");
    } else if (state.type == seb::shell::taskbar::platform::NetworkState::Type::Wireless) {
        if (state.status == seb::shell::taskbar::platform::NetworkState::Status::Connected) {
            tooltip = QStringLiteral("Connected to %1").arg(state.activeConnection);
        } else if (state.status == seb::shell::taskbar::platform::NetworkState::Status::Connecting) {
            tooltip = QStringLiteral("Connecting to wireless network");
        } else {
            tooltip = QStringLiteral("Wireless network disconnected");
        }
    }

    button_->setEnabled(state.available);
    button_->setToolTip(tooltip);
    button_->setIconPath(iconForNetwork(state));
}

}  // namespace seb::ui::taskbar
