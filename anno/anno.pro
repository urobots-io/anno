QT       += core gui network qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Use Precompiled headers (PCH)
PRECOMPILED_HEADER = stdafx.h

# generate git_rev.h
versionTarget.target = git_rev
win32: versionTarget.commands += call ../anno/gen_git_rev_windows.bat
else: versionTarget.commands += sh ../anno/gen_git_rev_x.sh
versionTarget.depends =
QMAKE_EXTRA_TARGETS += versionTarget
PRE_TARGETDEPS += git_rev

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AboutDialog.cpp ApplicationModel.cpp CircleLabel.cpp ColorDisplayWidget.cpp CreateLabelFileModelCommand.cpp \
    CustomPropertiesEditorTableItemDelegate.cpp \
    CustomPropertiesEditorTableModel.cpp \
    CustomPropertyTableItemDelegate.cpp CustomPropertyTableModel.cpp DeleteAllLabelsFileModelCommand.cpp \
    DeleteLabelFileModelCommand.cpp Desktop3dWindow.cpp DesktopWidget.cpp ElidedLabelWidget.cpp ErrorsListDialog.cpp FileModel.cpp Highlighter.cpp \
    FileTreeElement.cpp FilenamesEditorWidget.cpp \
    ImageLoader.cpp ImageModel.cpp ImageSettingsWidgetQt.cpp Label.cpp LabelCategory.cpp LabelDefinition.cpp LabelDefinitionPropertiesWidget.cpp \
    ImagePropertiesTableModel.cpp \
    ImagePropertiesWidget.cpp \
    ImageModelQt.cpp \
    LabelDefinitionPropertiesDialog.cpp \
    LabelDefinitionsTreeModel.cpp LabelHandle.cpp LabelPropertiesWidget.cpp LocalFilesystem.cpp main.cpp \
    MainWindow.cpp messagebox.cpp ModifyLabelCategoryFileModelCommand.cpp ModifyLabelGeometryFileModelCommand.cpp \
    ModifyLabelTextFileModelCommand.cpp OrientedCircleLabel.cpp OrientedPointLabel.cpp OrientedRectLabel.cpp PointCloudDisplayWidget.cpp \
    PointLabel.cpp PolygonLabel.cpp PolylineLabel.cpp ProjectDefinitionsDialog.cpp ProjectSettingsWidget.cpp \
    PropertyTableModel.cpp PropertyTableItemDelegate.cpp \
    PropertyDatabase.cpp qjson_helpers.cpp RecentActionsList.cpp RectLabel.cpp rest.cpp \
    RestDatasetFilesystem.cpp SharedPropertiesEditorTableModel.cpp ScriptPainter.cpp settings.cpp SourcePicturesTreeModel.cpp \
    StartupDialog.cpp \
    stdafx.cpp Serialization.cpp StampPropertiesEditorTableModel.cpp \
    SourcePicturesWidget.cpp ToolLabel.cpp ToolboxProxyModel.cpp ToolboxWidget.cpp win_helpers.cpp \
    triangulation/construct.c triangulation/misc.c triangulation/monotone.c triangulation/tri.c triangulation/xtime.c

HEADERS += \
    CustomPropertiesEditorTableItemDelegate.h \
    ImagePropertiesTableModel.h \
    ImagePropertiesWidget.h \
    LabelDefinitionPropertiesDialog.h \
    StartupDialog.h \
    AboutDialog.h ApplicationModel.h ArcBall.h CircleLabel.h ColorDisplayWidget.h ColoredVertexData.h \
    ColorTransformer.h \
    CustomPropertiesEditorTableModel.h \
    CreateLabelFileModelCommand.h CustomProperty.h CustomPropertyTableItemDelegate.h CustomPropertyTableModel.h \
    DeleteAllLabelsFileModelCommand.h DeleteLabelFileModelCommand.h Desktop3dWindow.h DesktopWidget.h ElidedLabelWidget.h ErrorsListDialog.h \
    FilenamesEditorWidget.h \
    FileModel.h FilesystemInterface.h FileTreeItemInfo.h geometry.h Highlighter.h ImageConverter.h ImageData.h ImageLoader.h \
    FileTreeElement.h \
    ImageModel.h ImageSettingsWidgetQt.h implement_q_property.h Label.h LabelCategory.h LabelDefinition.h LabelDefinitionPropertiesWidget.h \
    LabelDefinitionsTreeModel.h LabelFactory.h LabelHandle.h LabelPropertiesWidget.h LabelType.h LocalFilesystem.h \
    MainWindow.h messagebox.h ModifyLabelCategoryFileModelCommand.h ModifyLabelGeometryFileModelCommand.h \
    ModifyLabelTextFileModelCommand.h OrientedCircleLabel.h OrientedPointLabel.h OrientedRectLabel.h PaintInfo.h PointCloudDisplayShaders.h \
    PointCloudDisplayWidget.h PointLabel.h PolygonLabel.h PolylineLabel.h ProjectDefinitionsDialog.h ProjectSettingsWidget.h \
    PropertyDatabase.h ProxyLabel.h qjson_helpers.h RecentActionsList.h RectLabel.h rest.h RestDatasetFilesystem.h \
    PropertyTableModel.h PropertyTableItemDelegate.h \
    ScriptPainter.h settings.h SharedPropertyDefinition.h SharedPropertiesEditorTableModel.h SourcePicturesTreeModel.h SourcePicturesWidget.h \
    stdafx.h Serialization.h StampPropertiesEditorTableModel.h \
    ToolboxProxyModel.h ToolboxWidget.h ToolLabel.h win_helpers.h WorldInfo.h \
    triangulation/xtime.c triangulation/interface.h triangulation/triangulate.h triangulation/xtime.h

FORMS += \
    AboutDialog.ui Desktop3dWindow.ui \
    ErrorsListDialog.ui ImageSettingsWidgetQt.ui LabelDefinitionPropertiesWidget.ui \
    ImagePropertiesWidget.ui \
    LabelDefinitionPropertiesDialog.ui \
    LabelPropertiesWidget.ui MainWindow.ui ProjectDefinitionsDialog.ui \
    ProjectSettingsWidget.ui SourcePicturesWidget.ui ToolboxWidget.ui \
    StartupDialog.ui

RESOURCES += MainWindow.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Windows libs
win32  {
    LIBS += Ole32.lib
    LIBS += User32.lib
}
