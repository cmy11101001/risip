/***********************************************************************************
**    Copyright (C) 2016  Petref Saraci
**    http://risip.io
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You have received a copy of the GNU General Public License
**    along with this program. See LICENSE.GPLv3
**    A copy of the license can be found also here <http://www.gnu.org/licenses/>.
**
************************************************************************************/

#include "risip.h"

#include "risipglobals.h"
#include "risipaccount.h"
#include "risipbuddy.h"
#include "risipcall.h"
#include "risipaccountconfiguration.h"
#include "risipmedia.h"
#include "risipmessage.h"
#include "risipmodels.h"
#include "risipcallmanager.h"
#include "risipcontactmanager.h"

#include <QQmlEngine>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QSortFilterProxyModel>

static QObject *risipSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return Risip::instance();
}

static QObject *risipCallManagerSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return RisipCallManager::instance();
}

static QObject *risipContactManagerSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return RisipContactManager::instance();
}

Risip *Risip::m_risipInstance = NULL;
Risip *Risip::instance()
{
    if(!m_risipInstance)
        m_risipInstance = new Risip;

    return m_risipInstance;
}

Risip::Risip(QObject *parent)
    :QObject (parent)
    ,m_accounts()
    ,m_defaultAccountUri()
    ,m_defaultAccountAlways(true)
{

}

Risip::~Risip()
{
    //FIXME do not call this here - better handling
    saveSettings();

    while (!m_accounts.isEmpty())
        delete m_accounts.take(m_accounts.keys().takeFirst());

    m_sipEndpoint.stop();
}

QQmlListProperty<RisipAccount> Risip::accounts()
{
    QList<RisipAccount *> allAccounts = m_accounts.values();
    return QQmlListProperty<RisipAccount>(this, allAccounts);
}

QStringList Risip::accountNames() const
{
    return m_accounts.keys();
}

RisipContactManager *Risip::contactManager() const
{
    return RisipContactManager::instance();
}

RisipCallManager *Risip::callManager() const
{
    return RisipCallManager::instance();
}

RisipEndpoint *Risip::sipEndpoint()
{
    return &m_sipEndpoint;
}

RisipAccount *Risip::defaultAccount()
{
    return accountForUri(m_defaultAccountUri);
}

bool Risip::firstRun() const
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QVariant val = settings.value(RisipSettingsParam::FirstRun);
    if(val.isNull())
        return true;

    return settings.value(RisipSettingsParam::FirstRun).toBool();
}

bool Risip::defaultAccountAlways() const
{
    return m_defaultAccountAlways;
}

/**
 * @brief Risip::setDefaultAccountAlways
 * @param always
 *
 * Setting this to true will enable the call manager to use always the default account set
 * in Risip::defaultAccount @see Risip::setDefaultAccount
 */

void Risip::setDefaultAccountAlways(bool always)
{
    if(m_defaultAccountAlways != always) {
        m_defaultAccountAlways = always;
        emit defaultAccountAlwaysChanged(m_defaultAccountAlways);
    }
}

void Risip::setDefaultAccount(const QString &uri)
{
    if(m_defaultAccountUri != uri) {
        m_defaultAccountUri = uri;

        if(m_defaultAccountAlways) {
            RisipCallManager::instance()->setActiveAccount(accountForUri(m_defaultAccountUri));
            RisipContactManager::instance()->setActiveAccount(accountForUri(m_defaultAccountUri));
        }

        emit defaultAccountChanged(accountForUri(m_defaultAccountUri));
    }
}

void Risip::registerToQml()
{
    qmlRegisterSingletonType<Risip>(RisipSettingsParam::QmlUri, 1, 0, "Risip", risipSingletonProvider);
    qmlRegisterSingletonType<RisipCallManager>(RisipSettingsParam::QmlUri, 1, 0,
                                               "RisipCallManager", risipCallManagerSingletonProvider);
    qmlRegisterSingletonType<RisipContactManager>(RisipSettingsParam::QmlUri, 1, 0,
                                               "RisipContactManager", risipContactManagerSingletonProvider);

    qmlRegisterType<RisipEndpoint>(RisipSettingsParam::QmlUri, 1, 0, "RisipEndpoint");
    qmlRegisterType<RisipAccount>(RisipSettingsParam::QmlUri, 1, 0, "RisipAccount");
    qmlRegisterType<RisipBuddy>(RisipSettingsParam::QmlUri, 1, 0, "RisipBuddy");
    qmlRegisterType<RisipCall>(RisipSettingsParam::QmlUri, 1, 0, "RisipCall");
    qmlRegisterType<RisipAccountConfiguration>(RisipSettingsParam::QmlUri, 1, 0, "RisipAccountConfiguration");
    qmlRegisterType<RisipMedia>(RisipSettingsParam::QmlUri, 1, 0, "RisipMedia");
    qmlRegisterType<RisipMessage>(RisipSettingsParam::QmlUri, 1, 0, "RisipMessage");
    qmlRegisterType<RisipCallHistoryModel>(RisipSettingsParam::QmlUri, 1, 0, "RisipCallHistoryModel");
    qmlRegisterType<RisipBuddiesModel>(RisipSettingsParam::QmlUri, 1, 0, "RisipBuddiesModel");
    qmlRegisterType<RisipContactHistoryModel>(RisipSettingsParam::QmlUri, 1, 0, "RisipContactHistoryModel");

    qmlRegisterType<QSortFilterProxyModel>(RisipSettingsParam::QmlUri, 1, 0, "QSortFilterProxyModel");
}

RisipAccount *Risip::accountForUri(const QString &accountUri)
{
    if(!accountUri.isEmpty() && m_accounts.contains(accountUri))
        return m_accounts[accountUri];

    return new RisipAccount(this);
}

RisipAccount *Risip::accountForConfiguration(RisipAccountConfiguration *configuration)
{
    if(configuration) {
        RisipAccount *account;
        //return an existing account and update its configuration too
        if(m_accounts.contains(configuration->uri())) {
            account = m_accounts[configuration->uri()];
            account->setConfiguration(configuration);
            return account;
        }
    }

    return createAccount(configuration);
}

RisipAccount *Risip::createAccount(RisipAccountConfiguration *configuration)
{
    if(!configuration)
        return new RisipAccount(this);

    if(!configuration->valid())
        return new RisipAccount(this);

    //create a new account with the given configuration
    RisipAccount *account = new RisipAccount(this);
    account->setSipEndPoint(sipEndpoint());
    account->setConfiguration(configuration);
    m_accounts[account->configuration()->uri()] = account;

    RisipContactManager::instance()->createModelsForAccount(account);
    RisipCallManager::instance()->createModelsForAccount(account);

    emit accountsChanged();
    emit accountNamesChanged();

    return account;
}

bool Risip::removeAccount(QString &accountUri)
{
    if(m_accounts.contains(accountUri)) {
        m_accounts.take(accountUri)->deleteLater();
        RisipContactManager::instance()->removeModelsForAccount(accountForUri(accountUri));
        RisipCallManager::instance()->removeModelsForAccount(accountForUri(accountUri));
        return true;
    }

    return false;
}

bool Risip::removeAccount(RisipAccountConfiguration *configuration)
{
    QString accountUri;
    if(configuration)
        accountUri = configuration->uri();
    return removeAccount(accountUri);
}

bool Risip::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    int totaltAccounts = settings.value(RisipSettingsParam::TotalAccounts).toInt();

    RisipAccountConfiguration *configuration = NULL;
    settings.beginGroup(RisipSettingsParam::AccountGroup);
    for(int i=0; i<totaltAccounts; ++i) {
        settings.beginReadArray(QString("account" + QString::number(i))); //settings array for account

        configuration = new RisipAccountConfiguration;
        configuration->setUri(settings.value(RisipSettingsParam::Uri).toString());
        configuration->setUserName(settings.value(RisipSettingsParam::Username).toString());
        configuration->setPassword(settings.value(RisipSettingsParam::Password).toString());
        configuration->setNetworkProtocol(settings.value(RisipSettingsParam::NetworkType).toInt());
        configuration->setServerAddress(settings.value(RisipSettingsParam::ServerAddress).toString());
        configuration->setProxyServer(settings.value(RisipSettingsParam::ProxyServer).toString());
        configuration->setLocalPort(settings.value(RisipSettingsParam::LocalPort).toInt());
        configuration->setRandomLocalPort(settings.value(RisipSettingsParam::RandomLocalPort).toInt());
        configuration->setScheme(settings.value(RisipSettingsParam::Scheme).toString());

        RisipAccount *acc = createAccount(configuration); //creating account`
        acc->setAutoSignIn(settings.value(RisipSettingsParam::AutoSignIn).toBool());
        settings.endArray(); //end array
    }

    //TODO setting default account always when reading settings ??
//    setDefaultAccount(settings.value(RisipSettingsParam::DefaultAccount).toString());
    return true;
}

bool Risip::saveSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(RisipSettingsParam::TotalAccounts, m_accounts.size());
    settings.setValue(RisipSettingsParam::FirstRun, true); //FIXME always to true for testing
    settings.setValue(RisipSettingsParam::DefaultAccount, defaultAccount()->configuration()->uri());

    RisipAccount *account = NULL;
    settings.beginGroup(RisipSettingsParam::AccountGroup);
    for(int i=0; i<m_accounts.count(); ++i) {
        settings.beginWriteArray(QString("account" + QString::number(i)), 9); //settings array for account
        account = m_accounts[m_accounts.keys()[i]];
        settings.setValue(RisipSettingsParam::Uri, m_accounts.keys()[i]);
        settings.setValue(RisipSettingsParam::Username, account->configuration()->userName());
        settings.setValue(RisipSettingsParam::Password, account->configuration()->password());
        settings.setValue(RisipSettingsParam::NetworkType, account->configuration()->networkProtocol());
        settings.setValue(RisipSettingsParam::ServerAddress, account->configuration()->serverAddress());
        settings.setValue(RisipSettingsParam::ProxyServer, account->configuration()->proxyServer());
        settings.setValue(RisipSettingsParam::Scheme, account->configuration()->scheme());
        settings.setValue(RisipSettingsParam::LocalPort, account->configuration()->localPort());
        settings.setValue(RisipSettingsParam::RandomLocalPort, account->configuration()->randomLocalPort());
        settings.setValue(RisipSettingsParam::AutoSignIn, account->autoSignIn());
        settings.endArray(); //end settings array for account
    }

    settings.endGroup();
    return true;
}

bool Risip::resetSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.clear();
    settings.sync();
    return true;
}