/***************************************************************************
 *   Tor Download Manager (TDM) is an internet download manager that sends *
 *   all its traffic through the TOR network.                              *
 *                                                                         *
 *   Copyright (C) 2020 Darien Alonso Camacho                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 **************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QStringList>
#include <QMainWindow>
#include <QtNetwork>
#include <QTreeWidget>
#include "downloadtask.h"

QT_BEGIN_NAMESPACE
class QAction;
class QCloseEvent;
class QLabel;
class QProgressDialog;
class QSlider;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    QSize sizeHint() const override;
    static QString saveFileName(const QUrl &url);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    bool usingTORProxy();
    void loadSettings();
    void saveSettings();

    bool addUrl();
    void removeDownloadTask();

    void about();
    void setActionsEnabled();

    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();
    void finishedHead();
    void pause();

signals:
    void finished();

private:
    bool isHttpRedirect() const;
    void reportRedirect();
    bool addUrl(const QString &fileName, const QString &destinationFolder);
    void addDownloadListViewItem();

    QTreeWidget *downloadListView;
    QAction *startDownloadAction;
    QAction *pauseDownloadAction;
    QAction *removeDownloadAction;

    QString lastDirectory;

    QNetworkAccessManager manager;
    QList<DownloadTask*> downloadList;
    QNetworkReply *currentDownload = nullptr;
    QFile output;
    QTime downloadTime;

    QNetworkProxy proxy;

    bool acceptRanges;
    //A download task is running
    bool downloading;
    //The index of the current download
    int downloadTaskIndex;

    QNetworkReply *headReply;
};

#endif
