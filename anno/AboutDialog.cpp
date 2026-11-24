// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "AboutDialog.h"
#include "git_info.h"
#include "product_info.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);



    ui.copyright_label->setText(ANNO_PRODUCT_LEGAL_COPYRIGHT);    
    
    ui.repo_url_label->setText(git_info::URL);
    ui.revision_label->setText(git_info::RepositoryVersion().c_str());
    
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
