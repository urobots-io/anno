// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "AboutDialog.h"
#include "git_info.h"
#include "product_info.h"

namespace {
// web page of a repository: https://host/path.git or git@host:path.git -> https://host/path
QString RepositoryWebUrl(QString url) {
    url = url.trimmed();
    if (url.startsWith("git@")) {
        url = url.mid(4);
        auto colon = url.indexOf(':');
        if (colon > 0) {
            url[colon] = '/';
        }
        url = "https://" + url;
    }
    if (url.endsWith(".git")) {
        url.chop(4);
    }
    return url;
}

// shows the url as a link which opens in the default browser
void SetLink(QLabel *label, const QString &url) {
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        label->setText(url);
        return;
    }
    label->setTextFormat(Qt::RichText);
    label->setText(QString("<a href=\"%1\">%2</a>").arg(url.toHtmlEscaped(), url.toHtmlEscaped()));
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
}
}

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);



    ui.copyright_label->setText(ANNO_PRODUCT_LEGAL_COPYRIGHT);    
    
    SetLink(ui.repo_url_label, RepositoryWebUrl(git_info::URL));
    SetLink(ui.website_label, ui.website_label->text());
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
