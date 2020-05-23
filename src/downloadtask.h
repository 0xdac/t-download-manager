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

#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QUrl>
#include <QFileInfo>

class DownloadTask
{
public:
    DownloadTask(QUrl url);
    QUrl url();
    QString fileName();

    void setFileSize(qint64 size);
    qint64 fileSize() const;
    QString fileSizeToString() const;

    void setAcceptRanges(bool accept);
    bool acceptRanges() const;

    void setStatus(int status);
    int status() const;

private:
    QUrl _url;
    QString _fileName;

    //Resume capability
    bool _acceptRanges;

    qint64 totalSize;

    //Percent
    int _status;
};

#endif // DOWNLOADTASK_H
