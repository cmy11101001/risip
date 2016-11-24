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
#ifndef RISIPBUDDYMODELS_H
#define RISIPBUDDYMODELS_H

#include <QAbstractListModel>
#include <QQmlListProperty>
#include "risipglobals.h"

class QSortFilterProxyModel;
class RisipAccount;
class RisipBuddy;
class RisipCall;
class RisipPhoneContact;
class RisipPhoneNumber;

class RisipAbstractBuddyModel: public QAbstractListModel
{
    Q_OBJECT
public:
    enum BuddyRole {
        UriRole = Qt::UserRole + 1,
        ContactRole,
        TypeRole
    };

    explicit RisipAbstractBuddyModel(QObject *parent = NULL);
    virtual ~RisipAbstractBuddyModel();

    Q_ENUM(BuddyRole)
    Q_PROPERTY(RisipAccount * account READ account WRITE setAccount NOTIFY accountChanged)
    Q_PROPERTY(QSortFilterProxyModel * proxy READ proxy WRITE setProxy NOTIFY proxyChanged)

    RisipAccount *account() const;
    void setAccount(RisipAccount *account);

    QSortFilterProxyModel *proxy() const;
    void setProxy(QSortFilterProxyModel *proxy);

    // Basic functionality:
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual void addBuddy(RisipBuddy *buddy);
    virtual void removeBuddy(RisipBuddy *buddy);

Q_SIGNALS:
    void accountChanged(RisipAccount *account);
    void proxyChanged(QSortFilterProxyModel *proxy);

private:
    RisipAccount *m_account;
    QSortFilterProxyModel *m_proxy;
    QList<RisipBuddy *> m_buddies;
};

class RisipBuddiesModel: public RisipAbstractBuddyModel
{
    Q_OBJECT
public:
    explicit RisipBuddiesModel(QObject *parent = NULL);
    ~RisipBuddiesModel();
};

class RisipContactHistoryModel: public RisipAbstractBuddyModel
{
    Q_OBJECT
public:
    explicit RisipContactHistoryModel(QObject *parent = NULL);
    ~RisipContactHistoryModel();
};

class RisipCallHistoryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum CallDataRole {
        CallContactRole = Qt::UserRole + 1, //callContact
        CallDirectionRole, // callDirection
        CallDurationRole, // callDuration
        CallTimestampRole // callTimestamp
    };

    Q_PROPERTY(RisipAccount * account READ account WRITE setAccount NOTIFY accountChanged)
    Q_PROPERTY(QSortFilterProxyModel * proxy READ proxy WRITE setProxy NOTIFY proxyChanged)

    explicit RisipCallHistoryModel(QObject *parent = 0);
    ~RisipCallHistoryModel();

    RisipAccount *account();
    void setAccount(RisipAccount *account);

    QSortFilterProxyModel *proxy() const;
    void setProxy(QSortFilterProxyModel *proxy);

    // Basic functionality:
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void addCallRecord(RisipCall *call);
    void removeCallRecord(RisipCall *call);

Q_SIGNALS:
    void accountChanged(RisipAccount *account);
    void proxyChanged(QSortFilterProxyModel *proxy);

private:
    RisipAccount *m_account;
    QSortFilterProxyModel *m_proxy;
    QList<RisipCall *> m_calls;
};

class RisipPhoneContactsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum RisipPhoneContactDataRole {
        ContactId = Qt::UserRole + 1,
        FullName,
        Initials,
        PhoneNumberList
    };

    Q_PROPERTY(QSortFilterProxyModel * proxy READ proxy WRITE setProxy NOTIFY proxyChanged)

    explicit RisipPhoneContactsModel(QObject *parent = NULL);
    ~RisipPhoneContactsModel();

    QSortFilterProxyModel *proxy() const;
    void setProxy(QSortFilterProxyModel *proxy);

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE RisipPhoneContact *contactForIndex(int index);

public Q_SLOTS:
    void addContact(RisipPhoneContact *contact);
    void removeContact(RisipPhoneContact *contact);

Q_SIGNALS:
    void proxyChanged(QSortFilterProxyModel *proxy);

private:
    QList<RisipPhoneContact *> m_phoneContacts;
    QSortFilterProxyModel *m_proxy;
};

class RisipPhoneNumbersModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum RisipPhoneNumbersModelDataRole {
        FullNumber = Qt::UserRole + 1,
        CountryPrefix,
        RegionPrefix,
        CountryCode,
        CountryName,
        Number,
        RawNumber
    };

    Q_PROPERTY(RisipPhoneContact * phoneContact READ phoneContact WRITE setPhoneContact NOTIFY phoneContactChanged)

    explicit RisipPhoneNumbersModel(QObject *parent = 0);
    ~RisipPhoneNumbersModel();

    RisipPhoneContact *phoneContact() const;
    void setPhoneContact(RisipPhoneContact *contact);

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

Q_SIGNALS:
    void phoneContactChanged(RisipPhoneContact *contact);

private:
    RisipPhoneContact *m_phoneContact;
};

class RisipCountryRatesModel: public QAbstractListModel
{
    Q_OBJECT

public:

    enum RisipCountryRateDataRole {
        CountryName = Qt::UserRole +1,
        CountryCode,
        CountryPrefix,
        CountryRate,
        ValidFromDate,
        ValidTillDate
    };

    Q_PROPERTY(QSortFilterProxyModel * proxy READ proxy WRITE setProxy NOTIFY proxyChanged)

    explicit RisipCountryRatesModel(QObject *parent = 0);
    ~RisipCountryRatesModel();

    QSortFilterProxyModel *proxy() const;
    void setProxy(QSortFilterProxyModel *proxy);

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

Q_SIGNALS:
    void proxyChanged(QSortFilterProxyModel *proxy);

private:
    QSortFilterProxyModel *m_proxy;
    QList<Country> m_allCountries;
};
#endif // RISIPBUDDYMODELS_H
