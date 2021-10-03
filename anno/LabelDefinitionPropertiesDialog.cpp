#include "LabelDefinitionPropertiesDialog.h"

using namespace std;

LabelDefinitionPropertiesDialog::LabelDefinitionPropertiesDialog(shared_ptr<LabelDefinition> definition, shared_ptr<LabelDefinitionsTreeModel> definitions, QWidget *parent)
: QDialog(parent)
, definition_(definition)
, definitions_(definitions)
{
    ui.setupUi(this);
}

LabelDefinitionPropertiesDialog::~LabelDefinitionPropertiesDialog()
{
}

