/// macros to implement simple Q_PROPERTY functions
/// based on following naming convention
/// property name: property
/// name of the variable which holds property value: property_
/// READ function name: get_property
/// WRITE function name: set_property
/// NOTIFY function name: property_changed

#pragma once

/// get_%property% implementation
#define IMPLEMENT_Q_PROPERTY_READ(property_name) \
const decltype(property_name##_) & get_##property_name() const { return property_name##_; } 

/// get_%property% implementation, where property itself is a pointer to the class memeber
#define IMPLEMENT_Q_PROPERTY_READ_PTR(property_name) \
decltype(property_name##_)* get_##property_name() { return &property_name##_; } 

/// get_%property% implementation using mutex 
#define IMPLEMENT_Q_PROPERTY_READ_TS(property_name, mutex) \
const decltype(property_name##_) & get_##property_name() const { QMutexLocker locker(&const_cast<QMutex&>(mutex)); return property_name##_; } 

/// set_%property% implementation, triggers notification only if value was changed
#define IMPLEMENT_Q_PROPERTY_WRITE(type, property_name) \
void set_##property_name(type value) { if (property_name##_ != value) { \
    property_name##_ = value; emit property_name##_changed(value); } } 

/// set_%property% implementation using mutes, triggers notification only if value was changed
#define IMPLEMENT_Q_PROPERTY_WRITE_TS(type, property_name, mutex) \
void set_##property_name(type value) { \
{ QMutexLocker locker(&mutex); \
  if (property_name##_ == value) return; \
  property_name##_ = value; \
} \
emit property_name##_changed(value); } 

/// set_%property% implementation, always writes value and triggers notification (no value change check)
#define IMPLEMENT_Q_PROPERTY_WRITE_ALWAYS(type, property_name) \
void set_##property_name(type value) { \
    property_name##_ = value; emit property_name##_changed(value); } 

/// set_%property% declaration
#define DECLARE_Q_PROPERTY_WRITE(type, property_name) \
void set_##property_name(type value);

/// get_%property% implementation
/// Implementation for properties which are included in some structures
/// path - path to the property, i.e. for "data_.a.b.c.property" it shall be set to "data_.a.b.c" 
/// "_" is not added to the name of the variable 
#define IMPLEMENT_Q_PROPERTY_READ_S(path, property_name) \
const decltype(path.property_name) & get_##property_name() const { return path.property_name; } 

/// set_%property% implementation
/// Implementation for properties which are included in some structures
/// path - path to the property, i.e. for "data_.a.b.c.property" it shall be set to "data_.a.b.c" 
/// "_" is not added to the name of the variable 
#define IMPLEMENT_Q_PROPERTY_WRITE_S(path, type, property_name) \
void set_##property_name(type value) { if (path.property_name != value) { \
    path.property_name = value; emit property_name##_changed(value); } } 

