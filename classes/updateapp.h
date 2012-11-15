/***********************************************************************
*Copyright 2010-20XX by 7ymekk
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*
*   @author 7ymekk (7ymekk@gmail.com)
*
************************************************************************/


#ifndef UPDATEAPP_H
#define UPDATEAPP_H

#ifndef DELETE_IF_NOT_NULL
#define DELETE_IF_NOT_NULL(x) if(x != NULL){delete x; x = NULL;}
#endif

#include <QObject>
#include <QNetworkReply>
#include <QNetworkProxyFactory>
#include <QCoreApplication>
#include <QStringList>
#include "dialogs/logindialog.h"

class UpdateApp : public QObject
{
    Q_OBJECT
public:
    explicit UpdateApp(QObject *parent = 0);
    void checkUpdates();

private:
    QNetworkReply * reply;
    QNetworkAccessManager * updateMan;

signals:
    void updateState(bool, QString, QString);

public slots:
    void gotWWW(QNetworkReply *);
    void proxyAuthenticationRequired ( const QNetworkProxy & proxy, QAuthenticator * authenticator );
};

#endif // UPDATEAPP_H
