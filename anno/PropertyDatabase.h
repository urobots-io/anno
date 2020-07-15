#pragma once
#include "SharedPropertyDefinition.h"
#include <map>
#include <memory>

class PropertyDatabase {
    PropertyDatabase() {}
public:
    ~PropertyDatabase() {}

    static PropertyDatabase& Instance();

    int GetStateIndex() const { return state_index_; }
    void Modify() { state_index_++; }

    void Clear() {
        properties_.clear();
    }

    double GetCurrentValue(const std::string & name, double default_value);

protected:
    friend class LabelProperty;

    struct Value {
        // the value
        double value = 0.0;
        // additional parameter which might define how the variable was changed last time
        int iparam = 0;
        // each time when variable changes, this counter increments
        int update_counter = 0;
    };

    std::shared_ptr<Value> GetSharedValue(const std::string & name, double init_value, bool inject_init_value);

private:
    int state_index_ = 0;
    std::map<std::string, std::shared_ptr<Value>> properties_;
};

class LabelProperty {
public:
    void Connect(std::shared_ptr<SharedPropertyDefinition> definition, bool inject_my_value) {
        shared_ = nullptr;
        definition_ = definition;

        if (definition_) {
            shared_ = PropertyDatabase::Instance().GetSharedValue(
                definition_->name, 
                definition_->ToDatabaseValue(value_),
                inject_my_value);
        }
    }

    void Disconnect() {
        shared_ = nullptr;
        definition_ = nullptr;
    }

    bool IsShared() {
        return shared_.get() != nullptr;
    }   
    
    void set(double value, int iparam = 0) {
        value_ = value;
        if (shared_ && definition_) {
            shared_->value = definition_->ToDatabaseValue(value_);
            shared_->iparam = iparam;
            shared_->update_counter++;
            update_counter_ = shared_->update_counter;
            PropertyDatabase::Instance().Modify();
        }
    }    

    double get() const {
        if (shared_ && definition_) {
            return definition_->FromDatabaseValue(shared_->value);
        }
        else {
            return value_;
        }
    }

    int iparam() const {
        return shared_ ? shared_->iparam : 0;
    }

    // Returns 1 if shared variable was changed.
    // Otherwise, returns 0.
    // This is a helper function to be used in Label::UpdateSharedProperties
    int PullUpdate() {
        if (shared_ && shared_->update_counter != update_counter_) {
            update_counter_ = shared_->update_counter;
            return 1;
        }
        return 0;
    }

    std::shared_ptr<SharedPropertyDefinition> definition() const { return definition_; }

private:
    std::shared_ptr<PropertyDatabase::Value> shared_;
    std::shared_ptr<SharedPropertyDefinition> definition_;
    double value_ = 0.0;
    int update_counter_ = 0;
};
