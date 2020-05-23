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
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_REVISION 1

#include <QtWidgets>
#include "addurldialog.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qDebug()<< QSslSocket::sslLibraryBuildVersionString();
    /*
     * QNetworkProxy::Socks5Proxy.
     * Capabilities: TunnelingCapability, ListeningCapability, UdpTunnelingCapability, HostNameLookupCapability
     */
    proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(9150);
    QNetworkProxy::setApplicationProxy(proxy);

    downloading = false;

    // Initialize some static strings
    QStringList headers;
    headers << tr("File Name") << tr("Size") << tr("Status")
            << tr("Transfer Rate") << tr("Resume Capability") << tr("URL");

    // Main download list
    downloadListView = new QTreeWidget(this);
    downloadListView->setColumnCount(6);
    downloadListView->setHeaderLabels(headers);
    downloadListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    downloadListView->setAlternatingRowColors(true);
    downloadListView->setRootIsDecorated(false);
    setCentralWidget(downloadListView);

    // Set header resize modes and initial section sizes
    QHeaderView *header = downloadListView->header();
    header->resizeSection(0, 160);
    header->resizeSection(1, 55);
    header->resizeSection(2, 45);
    header->resizeSection(3, 90);
    header->resizeSection(4, 110);
    header->resizeSection(5, 200);

    // Create common actions
    QAction *newURLAction = new QAction(QIcon(":/icons/bottom.png"), tr("Add &new URL"), this);
    startDownloadAction = new QAction(QIcon(":/icons/player_play.png"), tr("&Start download"), this);
    pauseDownloadAction = new QAction(QIcon(":/icons/player_pause.png"), tr("&Pause download"), this);
    removeDownloadAction = new QAction(QIcon(":/icons/stop.png"), tr("&Remove download"), this);

    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newURLAction);
    fileMenu->addAction(pauseDownloadAction);
    fileMenu->addAction(removeDownloadAction);
    fileMenu->addSeparator();
    fileMenu->addAction(QIcon(":/icons/exit.png"), tr("E&xit"), this, SLOT(close()));

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, SLOT(about()));
    helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));

    // Top toolbar
    QToolBar *topBar = new QToolBar(tr("Tools"));
    addToolBar(Qt::TopToolBarArea, topBar);
    topBar->setMovable(false);
    topBar->addAction(newURLAction);
    topBar->addAction(removeDownloadAction);
    topBar->addAction(pauseDownloadAction);
    topBar->addAction(startDownloadAction);
    topBar->addSeparator();

#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    // Set up connections
    connect(downloadListView, SIGNAL(itemSelectionChanged()),
            this, SLOT(setActionsEnabled()));
    connect(newURLAction, SIGNAL(triggered()),
            this, SLOT(addUrl()));
    connect(startDownloadAction, SIGNAL(triggered()),
            this, SLOT(startNextDownload()));
    connect(pauseDownloadAction, SIGNAL(triggered()),
            this, SLOT(pause()));
    connect(removeDownloadAction, SIGNAL(triggered()),
            this, SLOT(removeDownloadTask()));

    // Load settings and start
    setWindowTitle(tr("Tor Download Manager"));
    setActionsEnabled();
    //QMetaObject::invokeMethod(this, "loadSettings", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "usingTORProxy", Qt::QueuedConnection);
}

bool MainWindow::usingTORProxy()
{
    if(proxy.type() != QNetworkProxy::Socks5Proxy || QNetworkProxy::applicationProxy().type() != QNetworkProxy::Socks5Proxy)
    {
        QMessageBox::warning(this, tr("Warning"), tr("You are not using the TOR network."));
        return false;
    }

    return true;
}

QSize MainWindow::sizeHint() const
{
    const QHeaderView *header = downloadListView->header();

    int width = fontMetrics().horizontalAdvance(tr("Downloading") + "  ");
    for (int i = 0; i < header->count(); ++i)
        width += header->sectionSize(i);

    return QSize(width, QMainWindow::sizeHint().height())
        .expandedTo(QApplication::globalStrut());
}

void MainWindow::loadSettings()
{
    // Load base settings (last working directory, upload/download limits).
    QSettings settings("TorDownloadManager", "List");
    lastDirectory = settings.value("LastDirectory").toString();
    if (lastDirectory.isEmpty())
        lastDirectory = QDir::currentPath();
    //int up = settings.value("UploadLimit").toInt();
    //int down = settings.value("DownloadLimit").toInt();

    // Resume all previous downloads.
    int size = settings.beginReadArray("List");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QByteArray resumeState = settings.value("resumeState").toByteArray();
        QString fileName = settings.value("sourceFileName").toString();
        QString dest = settings.value("destinationFolder").toString();

        if (addUrl(fileName, dest)) {

        }
    }
}

bool MainWindow::addUrl()
{
    // Show the "Add URL" dialog.
    AddUrlDialog *addURLDialog = new AddUrlDialog(this);
    addURLDialog->deleteLater();
    if (!addURLDialog->exec())
        return false;

    // Add the url to our list of downloads
    addUrl(addURLDialog->url(), addURLDialog->destinationFolder());

    return true;
}

void MainWindow::pause()
{
    if (currentDownload == NULL)
    {
        return;
    }
    //disconnect(currentDownload, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
    disconnect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    disconnect(currentDownload, SIGNAL(finished()), this,  SLOT(downloadFinished()));
    disconnect(currentDownload, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));

    currentDownload->abort();

    output.write(currentDownload->readAll());
    output.close();
    currentDownload = nullptr;
    this->downloading = false;
    this->setActionsEnabled();
}

void MainWindow::removeDownloadTask()
{

}


bool MainWindow::addUrl(const QString &url, const QString &destinationFolder)
{
    QNetworkAccessManager *headManager = new QNetworkAccessManager(this);

    QNetworkRequest currentRequest = QNetworkRequest(QUrl::fromEncoded(url.toLocal8Bit()));

    DownloadTask *task = new DownloadTask(QUrl::fromEncoded(url.toLocal8Bit()));
    downloadList.append(task);

    headReply = headManager->head(currentRequest);
    connect(headReply, SIGNAL(finished()), this, SLOT(finishedHead()));

    return true;
}

void MainWindow::saveSettings()
{
    // Prepare and reset the settings
    QSettings settings("TorDownloadManager", "List");
    settings.clear();

    settings.setValue("LastDirectory", lastDirectory);

    // Store data
    settings.beginWriteArray("List");
    settings.endArray();
    settings.sync();
}

void MainWindow::setActionsEnabled()
{
    QTreeWidgetItem *item = nullptr;
    DownloadTask *dTask = nullptr;
    int index = -1;

    if (!downloadListView->selectedItems().isEmpty())
    {
        item = downloadListView->selectedItems().first();
        index = downloadListView->indexOfTopLevelItem(item);
        dTask = downloadList.at(index);
    }

    startDownloadAction->setEnabled(item != 0 && !this->downloading && dTask && (dTask->status() < 100));
    pauseDownloadAction->setEnabled(item != 0 && this->downloading && dTask);
    removeDownloadAction->setEnabled(item != 0 && !this->downloading);
}

void MainWindow::about()
{
    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/icons/tdm.png"));

    QLabel *text = new QLabel;
    text->setWordWrap(true);
    QString aboutText = QString("<h3>Tor Download Manager %1.%2.%3</h3>"
                    "<p><b>Tor Download Manager</b> is an internet download manager "
                    "that sends all its traffic "
                    "through the TOR network. </p>"
                    "<p>Copyright (C) 2020 Darien Alonso</p>")
                    .arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_REVISION);
    text->setText(aboutText);

    QPushButton *quitButton = new QPushButton("OK");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setMargin(10);
    topLayout->setSpacing(10);
    topLayout->addWidget(icon);
    topLayout->addWidget(text);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(quitButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    QDialog about(this);
    about.setModal(true);
    about.setWindowTitle(tr("About Tor Download Manager"));
    about.setLayout(mainLayout);

    connect(quitButton, SIGNAL(clicked()), &about, SLOT(close()));

    about.exec();
}

void MainWindow::closeEvent(QCloseEvent *)
{

}

void MainWindow::startNextDownload()
{
    if(!usingTORProxy()){
        return;
    }

    if (downloadList.isEmpty()) {        
        emit finished();
        return;
    }

    QTreeWidgetItem *item = nullptr;
    this->downloadTaskIndex = -1;
    if (!downloadListView->selectedItems().isEmpty())
    {
        item = downloadListView->selectedItems().first();
        downloadTaskIndex = downloadListView->indexOfTopLevelItem(item);
    }

    DownloadTask *dTask = downloadList.at(this->downloadTaskIndex);
    if(dTask)
    {
        QUrl url = dTask->url();
        QNetworkRequest request(url);
        request.setRawHeader("Connection", "Keep-Alive");
        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

        QString filename = dTask->fileName();

        output.setFileName(filename);
        if (!output.open(QIODevice::ReadWrite | QIODevice::Append)) {
            return;                 // skip this download
        }

        //Is resuming?
        if (dTask->status() > 0 && dTask->acceptRanges())
        {
            qint64 downloadSizeAtPause = output.size();

            QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(downloadSizeAtPause) + "-";
            rangeHeaderValue += QByteArray::number(dTask->fileSize());

            request.setRawHeader("Range", rangeHeaderValue);
        }

        currentDownload = manager.get(request);

        connect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)),
                SLOT(downloadProgress(qint64,qint64)));
        connect(currentDownload, SIGNAL(finished()),
                SLOT(downloadFinished()));
        connect(currentDownload, SIGNAL(readyRead()),
                SLOT(downloadReadyRead()));

        this->downloading = true;
        downloadTime.start();
        this->setActionsEnabled();
    }
}

void MainWindow::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    // calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    }
    else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    }
    else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    DownloadTask *dTask = downloadList.at(this->downloadTaskIndex);
    if(dTask)
    {
        int percent = output.size() * 100 / dTask->fileSize();
        dTask->setStatus(percent);

        QTreeWidgetItem *item = downloadListView->topLevelItem(this->downloadTaskIndex);
        if (item)
            item->setText(2, QString::number(percent).append("%"));

        QString num;
        num.setNum(speed);
        num.append(" ").append(unit);
        downloadListView->topLevelItem(this->downloadTaskIndex)->setText(3, num);
    }
}

void MainWindow::downloadFinished()
{
    output.close();

    if (currentDownload->error()) {
        // download failed
        output.remove();
    }
    else {
        // let's check if it was actually a redirect
        if (isHttpRedirect()) {
            reportRedirect();
            output.remove();
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Download complete.");
            msgBox.exec();
        }
    }

    currentDownload->deleteLater();
    this->downloading = false;
    this->setActionsEnabled();
}

void MainWindow::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}

bool MainWindow::isHttpRedirect() const
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void MainWindow::reportRedirect()
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QUrl requestUrl = currentDownload->request().url();
    QTextStream(stderr) << "Request: " << requestUrl.toDisplayString()
                        << " was redirected with code: " << statusCode
                        << '\n';

    QVariant target = currentDownload->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!target.isValid())
        return;
    QUrl redirectUrl = target.toUrl();
    if (redirectUrl.isRelative())
        redirectUrl = requestUrl.resolved(redirectUrl);
    QTextStream(stderr) << "Redirected to: " << redirectUrl.toDisplayString()
                        << '\n';
}

QString MainWindow::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
        basename = "download";

    if (QFile::exists(basename)) {
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return basename;
}

void MainWindow::finishedHead()
{
    DownloadTask *lastAddedTask = nullptr;

    if(!downloadList.empty())
    {
        lastAddedTask = downloadList.at(downloadList.size() - 1);
    }

    if(lastAddedTask)
    {
        if (headReply->hasRawHeader("Accept-Ranges"))
        {
            QString qstrAcceptRanges = headReply->rawHeader("Accept-Ranges");
            acceptRanges = (qstrAcceptRanges.compare("bytes", Qt::CaseInsensitive) == 0);

            lastAddedTask->setAcceptRanges(acceptRanges);            
        }

        qint64 downloadTotal = headReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        lastAddedTask->setFileSize(downloadTotal);

        this->addDownloadListViewItem();
    }
}

void MainWindow::addDownloadListViewItem()
{
    if(!downloadList.empty())
    {
        DownloadTask *task = downloadList.last();
        if(task)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem();

            //Filename
            item->setText(0, task->fileName());
            item->setToolTip(0, tr("%1").arg(task->fileName()));
            //Size
            item->setText(1, task->fileSizeToString());
            //Status
            item->setText(2, "0%");
            //Transfer rate
            item->setText(3, "0.0 KB/s");
            //Resume capability
            item->setText(4, task->acceptRanges() ? "Yes":"No");
            //URL
            item->setText(5, task->url().toString());
            item->setToolTip(5, tr("%1").arg(task->url().toString()));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            //item->setTextAlignment(1, Qt::AlignHCenter);
            downloadListView->addTopLevelItem(item);
        }
    }
}
