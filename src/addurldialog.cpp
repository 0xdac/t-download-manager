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

#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QMetaObject>
#include "addurldialog.h"

AddUrlDialog::AddUrlDialog(QWidget *parent)
  : QDialog(parent, Qt::Sheet)
{
    ui.setupUi(this);

    /*connect(ui.browseDestination, SIGNAL(clicked()),
            this, SLOT(selectDestination()));*/
    connect(ui.urlString, SIGNAL(textChanged(QString)),
            this, SLOT(setUrl()));

    //ui.destinationFolder->setText(destinationDirectory = QDir::current().path());
    ui.urlString->setFocus();
}

void AddUrlDialog::selectDestination()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a destination directory"),
                                                    lastDestinationDirectory);
    if (dir.isEmpty())
        return;
    lastDestinationDirectory = destinationDirectory = dir;
    //ui.destinationFolder->setText(destinationDirectory);
    enableOkButton();
}

void AddUrlDialog::enableOkButton()
{
    ui.okButton->setEnabled(!ui.urlString->text().isEmpty());
}

void AddUrlDialog::setUrl()
{
    enableOkButton();
}

QString AddUrlDialog::url() const
{
    return ui.urlString->text();
}

QString AddUrlDialog::destinationFolder() const
{
    return this->destinationDirectory;
}
