#include "NavigationModel.h"

void NavigationModel::Clear() {
    paths_.clear();
    set_current_path({});
    set_can_back(false);
    set_can_forward(false);
}

void NavigationModel::SetPath(const QString & path) {   
    if (path.isEmpty()) {
        return;
    }

    while (index_ + 1 < paths_.size()) {
        paths_.removeLast();
    }

    paths_ << path;
    index_ = paths_.size() - 1;

    set_current_path(path);
    set_can_back(index_ > 0);
    set_can_forward(false);
     
}

void NavigationModel::Back() {
    if (index_ > 0) {
        index_--;
        set_current_path(paths_[index_]);
        set_can_back(index_ > 0);
        set_can_forward(true);
    }
}

void NavigationModel::Forward() {
    if (index_ + 1 < paths_.size()) {
        index_++;
        set_current_path(paths_[index_]);
        set_can_back(true);
        set_can_forward(index_ + 1 < paths_.size());
    }
}

