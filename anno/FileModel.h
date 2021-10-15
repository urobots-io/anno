#pragma once
#include "implement_q_property.h"
#include "Label.h"
#include <QObject>
#include <QUndoStack>

class FileModel : public QObject
{
    Q_OBJECT

public:
    FileModel(QString id);
    ~FileModel();

    Q_PROPERTY(QString id READ get_id WRITE set_id NOTIFY id_changed);
    Q_PROPERTY(bool is_modified READ get_is_modified WRITE set_is_modified NOTIFY is_modified_changed);
    
    QUndoStack *GetUndoStack() { return &undo_stack_; }

    // todo(ap) protect this
    std::vector<std::shared_ptr<Label>> labels_;

    // Undoable actions
    void DeleteAllLabels();    
    void PushCommand(QUndoCommand * command);

    std::shared_ptr<Label> CreateLabel(std::shared_ptr<Label>);
    std::shared_ptr<Label> DeleteLabel(std::shared_ptr<Label>);
    void ModifyLabelGeometry(std::shared_ptr<Label>, QStringList data = {});
    void ModifyLabelCategory(std::shared_ptr<Label>, int category);
    void ModifyLabelText(std::shared_ptr<Label>, QString text);

    void NotifyUpdate(bool clear_selection, std::shared_ptr<Label> label_to_select = {});
    void NotifyUpdate(std::shared_ptr<Label> label_to_select);

    void Delete(std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelCategory>);

    bool HaveLabels() const { return !labels_.empty(); }

    /// Return Proxy of the <label> or label itself, if it has no proxy
    std::shared_ptr<Label> GetOwner(std::shared_ptr<Label> label);

    /// Return Proxy of the label handle or label itself, if it has no proxy
    std::shared_ptr<Label> GetHandleOwner(std::shared_ptr<LabelHandle> handle);

    /// Create label of the shared category
    void CreateDefaultSharedLabel(std::shared_ptr<LabelCategory> category);
    
    /// Return shared indexes which are present on this file
    std::set<int> GetExistingSharedIndexes(std::shared_ptr<LabelDefinition> def);

    /// Definition gets new shared labels
    void UpdateDefinitionSharedLabels(std::shared_ptr<LabelDefinition> def, std::vector<std::shared_ptr<Label>>& shared_labels);

public slots:
    IMPLEMENT_Q_PROPERTY_WRITE(bool, is_modified);
    IMPLEMENT_Q_PROPERTY_WRITE(QString, id);

signals:        
    void is_modified_changed(bool);
    void rendering_required();
    void file_updated(bool);
    void id_changed(QString);
    void select_label_required(std::shared_ptr<Label>);

private:
    bool is_modified_ = false;
    QString id_;
    QUndoStack undo_stack_;

public:
    IMPLEMENT_Q_PROPERTY_READ(is_modified);
    IMPLEMENT_Q_PROPERTY_READ(id);
};

