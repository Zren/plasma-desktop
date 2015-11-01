/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "leavemodel.h"

// Qt
#include <QFileInfo>

// KDE
#include <KAuthorized>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QDebug>
#include <QIcon>
#include <Solid/PowerManagement>
#include <kworkspace.h>
#include <kdisplaymanager.h>
#include <kdirwatch.h>

// Local
#include "models.h"
#include "itemhandlers.h"

using namespace Kickoff;

class LeaveModel::Private
{
};

QStandardItem* LeaveModel::createStandardItem(const QString& url)
{
    //Q_ASSERT(KUrl(url).scheme() == "leave");
    QStandardItem *item = new QStandardItem();
    const QString basename = QFileInfo(url).baseName();
    if (basename == QLatin1String("logoutonly")) {
        item->setText(i18n("Log out"));
        item->setData(i18n("End session"), Kickoff::SubTitleRole);
        item->setData("system-log-out", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("lock")) {
        item->setText(i18n("Lock"));
        item->setData(i18n("Lock screen"), Kickoff::SubTitleRole);
        item->setData("system-lock-screen", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("switch")) {
        item->setText(i18n("Switch user"));
        item->setData(i18n("Start a parallel session as a different user"), Kickoff::SubTitleRole);
        item->setData("system-switch-user", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("shutdown")) {
        item->setText(i18n("Shut down"));
        item->setData(i18n("Turn off computer"), Kickoff::SubTitleRole);
        item->setData("system-shutdown", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("restart")) {
        item->setText(i18nc("Restart computer", "Restart"));
        item->setData(i18n("Restart computer"), Kickoff::SubTitleRole);
        item->setData("system-reboot", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("savesession")) {
        item->setText(i18n("Save Session"));
        item->setData(i18n("Save current session for next login"), Kickoff::SubTitleRole);
        item->setData("document-save", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("standby")) {
        item->setText(i18nc("Puts the system on standby", "Standby"));
        item->setData(i18n("Pause without logging out"), Kickoff::SubTitleRole);
        item->setData("system-suspend", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("suspenddisk")) {
        item->setText(i18n("Hibernate"));
        item->setData(i18n("Suspend to disk"), Kickoff::SubTitleRole);
        item->setData("system-suspend-hibernate", Kickoff::IconNameRole);
    } else if (basename == QLatin1String("suspendram")) {
        item->setText(i18n("Suspend"));
        item->setData(i18n("Suspend to RAM"), Kickoff::SubTitleRole);
        item->setData("system-suspend", Kickoff::IconNameRole);
    } else {
        item->setText(basename);
        item->setData(url, Kickoff::SubTitleRole);
    }

    item->setData(url, Kickoff::UrlRole);
    return item;
}

LeaveModel::LeaveModel(QObject *parent)
        : QStandardItemModel(parent)
        , d(0)
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Kickoff::SubTitleRole] = "subtitle";
    roles[Kickoff::UrlRole] = "url";
    roles[GroupNameRole] = "group";
    roles[Kickoff::IconNameRole] = "iconName";
    setRoleNames(roles);
    updateModel();
    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ProtocolHandler, QStringLiteral("leave"), new Kickoff::LeaveItemHandler);


    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/ksmserverrc";
    KDirWatch::self()->addFile(configFile);

    // Catch both, direct changes to the config file ...
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &LeaveModel::updateModel);
    // ... but also remove/recreate cycles, like KConfig does it
    connect(KDirWatch::self(), &KDirWatch::created, this, &LeaveModel::updateModel);
}

QVariant LeaveModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return i18n("Leave");
        break;
    default:
        return QVariant();
    }
}

void LeaveModel::updateModel()
{
    clear();

    // Session Options
    const QString session = i18n("Session");

    // Logout
    const bool canLogout = KAuthorized::authorizeKAction(QStringLiteral("logout")) && KAuthorized::authorize(QStringLiteral("logout"));
    if (canLogout) {
        QStandardItem *logoutOption = createStandardItem(QStringLiteral("leave:/logoutonly"));
        logoutOption->setData(session, Kickoff::GroupNameRole);
        appendRow(logoutOption);
    }

    // Lock
    if (KAuthorized::authorizeKAction(QStringLiteral("lock_screen"))) {
        QStandardItem *lockOption = createStandardItem(QStringLiteral("leave:/lock"));
        lockOption->setData(session, Kickoff::GroupNameRole);
        appendRow(lockOption);
    }

    // Save Session
    if (canLogout) {
        KConfigGroup c(KSharedConfig::openConfig(QStringLiteral("ksmserverrc"), KConfig::NoGlobals), "General");
        if (c.readEntry("loginMode") == QLatin1String("restoreSavedSession")) {
            QStandardItem *saveSessionOption = createStandardItem(QStringLiteral("leave:/savesession"));
            saveSessionOption->setData(session, Kickoff::GroupNameRole);
            appendRow(saveSessionOption);
        }
    }

    // Switch User
    if (KDisplayManager().isSwitchable() && KAuthorized::authorize(QStringLiteral("switch_user")))  {
        QStandardItem *switchUserOption = createStandardItem(QStringLiteral("leave:/switch"));
        switchUserOption->setData(session, Kickoff::GroupNameRole);
        appendRow(switchUserOption);
    }

    // System Options

//FIXME: the proper fix is to implement the KWorkSpace methods for Windows
#ifndef Q_WS_WIN
    const QString system = i18n("System");
    QSet< Solid::PowerManagement::SleepState > spdMethods = Solid::PowerManagement::supportedSleepStates();
    if (spdMethods.contains(Solid::PowerManagement::StandbyState)) {
        QStandardItem *standbyOption = createStandardItem(QStringLiteral("leave:/standby"));
        standbyOption->setData(system, Kickoff::GroupNameRole);
        appendRow(standbyOption);
    }

    if (spdMethods.contains(Solid::PowerManagement::SuspendState)) {
        QStandardItem *suspendramOption = createStandardItem(QStringLiteral("leave:/suspendram"));
        suspendramOption->setData(system, Kickoff::GroupNameRole);
        appendRow(suspendramOption);
    }

    if (spdMethods.contains(Solid::PowerManagement::HibernateState)) {
        QStandardItem *suspenddiskOption = createStandardItem(QStringLiteral("leave:/suspenddisk"));
        suspenddiskOption->setData(system, Kickoff::GroupNameRole);
        appendRow(suspenddiskOption);
    }

    if (canLogout) {
        if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot)) {
            // Restart
            QStandardItem *restartOption = createStandardItem(QStringLiteral("leave:/restart"));
            restartOption->setData(system, Kickoff::GroupNameRole);
            appendRow(restartOption);
        }

        if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeHalt)) {
            // Shutdown
            QStandardItem *shutDownOption = createStandardItem(QStringLiteral("leave:/shutdown"));
            shutDownOption->setData(system, Kickoff::GroupNameRole);
            appendRow(shutDownOption);
        }
    }
#endif
}

LeaveModel::~LeaveModel()
{
    delete d;
}


