#include "AboutDialog.h"
#include "version.h"
#include "product_info.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    std::string url = version::URL;
    const char *webaddress_end = ".com/";
    auto pos = url.find(webaddress_end);
    if (pos != std::string::npos)
        url.erase(url.begin(), url.begin() + pos + strlen(webaddress_end));

    ui.copyright_label->setText(ANNO_PRODUCT_LEGAL_COPYRIGHT);
    ui.svn_path_label->setText(tr("Repository %0").arg(url.c_str()));
    ui.svn_revision_label->setText(tr("Revision %0 (%1)").arg(version::rev_range).arg(version::modified));
    ui.version_label->setText(tr("Version %0").arg(ANNO_PRODUCT_VERSION_STRING));
    ui.build_time_label->setText(tr("Build time %0").arg(version::time_now));
#ifdef ANNO_USE_OPENCV
    ui.opencv_version_label->setText(tr("OpenCV Version %0").arg(CV_VERSION));
#else    
    ui.opencv_version_label->setText("-");
#endif

}

AboutDialog::~AboutDialog()
{
}
