/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "conversionpanel.h"
#include "ui_conversionpanel.h"

conversionPanel::conversionPanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::conversionPanel)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC // Damit es im Vordergrund bleibt, evtl. für windows auch sinnvoll, probiere es einfach mal unter windows aus
    setWindowFlags(windowFlags() | Qt::Popup);
#endif

    phantomChanges = false;
    ui->mpsSpin->setDecimals(3);
    ui->kphSpin->setDecimals(3);
    ui->mphSpin->setDecimals(3);
    ui->mpsSpin->setSuffix(QString(" m/s"));
    ui->kphSpin->setSuffix(QString(" km/h"));
    ui->mphSpin->setSuffix(QString(" mph"));

    ui->meterSpin->setDecimals(3);
    ui->feetSpin->setDecimals(3);
    ui->mileSpin->setDecimals(3);
    ui->meterSpin->setSuffix(QString(" m"));
    ui->feetSpin->setSuffix(QString(" ft"));
    ui->mileSpin->setSuffix(QString(" miles"));
}

conversionPanel::~conversionPanel()
{
    delete ui;
}

void conversionPanel::on_buttonBox_rejected()
{
    this->close();
}

void conversionPanel::on_buttonBox_accepted()
{
    this->close();
}

void conversionPanel::on_mpsSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    ui->kphSpin->setValue(arg1*3.6);
    ui->mphSpin->setValue(arg1*2.2369356);
    phantomChanges = false;
}

void conversionPanel::on_kphSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    ui->mpsSpin->setValue(arg1/3.6);
    ui->mphSpin->setValue(arg1*2.2369356/3.6);
    phantomChanges = false;
}

void conversionPanel::on_mphSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    ui->mpsSpin->setValue(arg1/2.2369356);
    ui->kphSpin->setValue(arg1/2.2369356*3.6);
    phantomChanges = false;
}

void conversionPanel::on_meterSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    ui->feetSpin->setValue(arg1/0.3048);
    ui->mileSpin->setValue(arg1/1609.34498);
    phantomChanges = false;
}

void conversionPanel::on_feetSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    ui->meterSpin->setValue(arg1*0.3048);
    ui->mileSpin->setValue(arg1*0.3048/1609.34498);
    phantomChanges = false;
}

void conversionPanel::on_mileSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    ui->meterSpin->setValue(arg1*1609.34498);
    ui->feetSpin->setValue(arg1/0.3048*1609.34498);
    phantomChanges = false;
}
