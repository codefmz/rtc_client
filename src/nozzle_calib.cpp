#include "nozzle_calib.h"
#include "ui_nozzlecalib.h"
#include "mainwindow.h"
#include "utils.h"
#include "webrtc.h"
#include <QVector3D>
#include <QDir>

NozzleCalib::NozzleCalib(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NozzleCalib)
{
    ui->setupUi(this);
}

NozzleCalib::~NozzleCalib()
{
    delete ui;
}
