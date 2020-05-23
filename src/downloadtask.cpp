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

#include "downloadtask.h"

DownloadTask::DownloadTask(QUrl url)
{
    this->_url = url;
    this->_fileName = QFileInfo(url.path()).fileName();
    this->_acceptRanges = false;
    this->_status = 0;
}

QUrl DownloadTask::url()
{
    return this->_url;
}

QString DownloadTask::fileName()
{
    return this->_fileName;
}

void DownloadTask::setFileSize(qint64 size)
{
    this->totalSize = size;
}

qint64 DownloadTask::fileSize() const
{
    return this->totalSize;
}

QString DownloadTask::fileSizeToString() const
{
    QString unit;
    double _size = this->totalSize;

    if (_size < 1024) {
        unit = "bytes";
    }
    else if (_size < 1024*1024) {
        _size /= 1024;
        unit = "kB";
    }
    else if (_size < 1024*1024*1024) {
        _size /= 1024*1024;
        unit = "MB";
    }
    else {
        _size /= 1024*1024*1024;
        unit = "GB";
    }

    return QString::number(_size, 'g', 3).append(" ").append(unit);
}

void DownloadTask::setAcceptRanges(bool accept)
{
    this->_acceptRanges = accept;
}

bool DownloadTask::acceptRanges() const
{
    return this->_acceptRanges;
}

void DownloadTask::setStatus(int status)
{
    this->_status = status;
}

int DownloadTask::status() const
{
    return this->_status;
}
