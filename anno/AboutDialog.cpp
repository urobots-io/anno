// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include "AboutDialog.h"
#include "git_rev.h"
#include "product_info.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);



    ui.copyright_label->setText(ANNO_PRODUCT_LEGAL_COPYRIGHT);    
    ui.revision_label->setText(tr("Revision %0").arg(QString::fromLatin1(git_rev).trimmed()));
    ui.version_label->setText(tr("Version %0").arg(ANNO_PRODUCT_VERSION_STRING));
    ui.build_time_label->setText(tr("Build time %0").arg(__DATE__ " " __TIME__));
#ifdef ANNO_USE_OPENCV
    ui.opencv_version_label->setText(tr("OpenCV Version %0").arg(CV_VERSION));
#else    
    ui.opencv_version_label->setText("-");
#endif

}

AboutDialog::~AboutDialog()
{
}
