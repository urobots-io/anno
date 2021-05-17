#pragma once
#include "ApplicationModel.h"
#include "FileModel.h"
#include "ImageLoader.h"
#include "ImageModel.h"
#include "implement_q_property.h"
#include <QJSEngine>
#include <QWidget>

class DesktopWidget : public QWidget
{
	Q_OBJECT

public:
	DesktopWidget(QWidget *parent);
    ~DesktopWidget() override;

    void Init(ApplicationModel *model);
		
    /// Status
    Q_PROPERTY(QString status READ get_status WRITE set_status NOTIFY status_changed)
    /// Image loading status
    Q_PROPERTY(bool is_loading_image READ get_is_loading_image WRITE set_is_loading_image NOTIFY is_loading_image_changed)
	/// Mouse cursor position in image coordinate system
    Q_PROPERTY(QPointF mouse_pos READ get_mouse_pos WRITE set_mouse_pos NOTIFY mouse_pos_changed)
	/// Image scale
    Q_PROPERTY(float world_scale READ get_world_scale WRITE set_world_scale NOTIFY world_scale_changed)
    /// Image scale power
    Q_PROPERTY(int world_scale_power READ get_world_scale_power WRITE set_world_scale_power NOTIFY world_scale_power_changed)
	/// Category which will be used to create labels in the creation mode
    Q_PROPERTY(std::shared_ptr<LabelCategory> category_for_creation READ get_category_for_creation WRITE set_category_for_creation NOTIFY category_for_creation_changed)
	/// Creation mode status
    Q_PROPERTY(bool is_creation_mode READ get_is_creation_mode WRITE set_is_creation_mode NOTIFY is_creation_mode_changed)
	/// Fit image to the widow 
    Q_PROPERTY(bool fit_to_view_on_load READ get_fit_to_view_on_load WRITE set_fit_to_view_on_load NOTIFY fit_to_view_on_load_changed)
	/// Selected label
    Q_PROPERTY(std::shared_ptr<Label> selected_label READ get_selected_label WRITE set_selected_label NOTIFY selected_label_changed)

    
	// begin: qt widget events
	void paintEvent(QPaintEvent *) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;	
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
	// end: qt widget events

	/// Model to control background image properties
    ImageModel *GetBackgroundImage() { return &image_; }

    /// Abort creation of object if any
    void AbortCreation();

    /// Current file model
    std::shared_ptr<FileModel> GetFile() const { return file_; }

private:
	QTransform GetWorldTransform() const;
	
	void SetMousePos(QMouseEvent*);

	std::shared_ptr<LabelHandle> FindClosestHandle(QPointF position);

	WorldInfo GetWorldInfo() const;
    void RefreshWorldScalePower() {
        world_scale_power_ = int(round(log(world_scale_) / log(world_scale_base_)));
    }

    void RenderHoveredHandle(QPainter & painter);
    void RenderMovingCross(QPainter & painter);
    void RenderSelectCross(QPainter & painter);
    void RenderCreationCross(QPainter & painter);

    // Render hight contrast cross from (0, 0)
    void RenderCross(QPainter & painter, int size, QColor color);

    // Create stamp label for selected <category_for_creation>
    void CreateStampLabel();
    
public slots:
    void SetFile(std::shared_ptr<FileModel>);
    void ImageLoaded();
	
    void OnRerenderFile();
    void OnSelectLabel(std::shared_ptr<Label>);
    void DeleteLabel(std::shared_ptr<Label>);

	void ResetWorldTransform();	
    void DeleteSelectedLabel();
	void MoveSelectedLabel(QPointF offset);
    void SetWorldScaleAndUpdate(double value, bool is_num_steps = true);

    void FitBackgroundToView();
    void OnImageScriptChanged(QString);

    void OnLabelDefinitionsChanged(std::shared_ptr<LabelDefinitionsTreeModel>);

    DECLARE_Q_PROPERTY_WRITE(std::shared_ptr<LabelCategory>, category_for_creation)
    DECLARE_Q_PROPERTY_WRITE(bool, is_creation_mode)
    IMPLEMENT_Q_PROPERTY_WRITE(bool, fit_to_view_on_load)

private slots:
    IMPLEMENT_Q_PROPERTY_WRITE(bool, is_loading_image)
    IMPLEMENT_Q_PROPERTY_WRITE(QPointF, mouse_pos)
    IMPLEMENT_Q_PROPERTY_WRITE(QString, status)
    DECLARE_Q_PROPERTY_WRITE(double, world_scale)
    DECLARE_Q_PROPERTY_WRITE(int, world_scale_power)
    DECLARE_Q_PROPERTY_WRITE(std::shared_ptr<Label>, selected_label)

signals:
	void mouse_pos_changed(QPointF);
    void world_scale_changed(double);
    void world_scale_power_changed(int);
	void category_for_creation_changed(std::shared_ptr<LabelCategory>);
	void is_creation_mode_changed(bool);
	void fit_to_view_on_load_changed(bool);
    void is_loading_image_changed(bool);
    void selected_label_changed(std::shared_ptr<FileModel>, std::shared_ptr<Label>);
    void status_changed(QString);
	    
private:
    void SetCategoryValueForSelectedLabel(int category_value);
    
    void ChangeCurrentCategory(int category_value);    

	bool AbortCreationMode();

    std::shared_ptr<Label> FindLabelUnderCursor();

    void StartExtraActionUnderCursor();

	enum class CursorMode {
		/// moving over the desktop with no action
		select,

        /// a handle was selected, its pointer shall be saved in <selected_handle_>
        move_handle_start,

		/// moving a handle, its pointer shall be saved in <selected_handle_>
		move_handle,

		/// creating an object of <category_for_creation_> type
		creation_start,

		/// creating an object of <category_for_creation_> type, its pointer is saved in <selected_label_>
		creation_in_progress,

		/// modifying the <selected_label_>: i.e. rotating oriented rect 
        /// or adding new points to the polygon
		modifying
	};
    
    void SetCursorMode(CursorMode mode);

private:
    /// File to edit
    std::shared_ptr<FileModel> file_;

	/// mouse position in background picture coordinates
	QPointF mouse_pos_;

	/// mouse position in window coordinate system
	QPoint mouse_pos_pixels_;

    /// "angle" of the mouse position, degrees
    /// defines initial rotation of the label for
    /// creation
    double mouse_angle_ = 0;

	/// current mode
	CursorMode cursor_mode_ = CursorMode::select;

	/// true, if creation of label will be finished with a next click
	bool is_creation_to_be_completed_ = false;		

	/// mouse cursor is over this handle
    std::shared_ptr<LabelHandle> hovered_handle_;

    /// mouse cursor is over this label
    std::shared_ptr<Label> hovered_label_;

	/// selected handle
    std::shared_ptr<LabelHandle> selected_handle_;

	/// selected label    
	std::shared_ptr<Label> selected_label_;

    /// label used as a stamp during creation
    std::shared_ptr<Label> stamp_label_;

	/// type of label which shall be created in creation mode        
	std::shared_ptr<LabelCategory> category_for_creation_;

	/// is creation mode
	bool is_creation_mode_ = false;

	/// scale and transform of the objects on the desktop
    double world_scale_ = 1;
    int world_scale_power_ = 0;
    const double world_scale_base_ = 1.1;
	QPointF world_offset_ = QPointF(1, 1);

	///the number of pixels on the screen that the selected label moves
	const int num_of_move_pixels_ = 1;
	bool is_need_to_undostack_move_label_by_arrows_ = true;

	/// fit loaded image to view when it is loaded by SetBackground
	bool fit_to_view_on_load_ = true;	

    /// fit loaded image to view when widow is resized
    bool fit_to_view_on_resize_ = true;

    /// Background image
    ImageModel image_;

	/// applicaion model to access project settings, loaders, converters, etc.
	ApplicationModel *model_ = nullptr;

    /// Loaders
	ImageLoader* current_loader_ = nullptr;
	std::vector<ImageLoader*> loaders_;

    /// Current status
    QString status_;

    /// Is loading of an image is in progress
    bool is_loading_image_ = false;

    /// Script engine to render custom label graphics
	QJSEngine js_engine_;

    /// Known state of the properties database
    /// Used to force labels to update their chared properties
    int props_db_state_on_paint_ = 0;    

    /// True, if system cursor (arrow) shall is hidden.
    bool hide_cursor_ = false;

    /// LButton was pressed on the empty area.
    bool lbutton_pressed_in_background_ = false;

public:
    IMPLEMENT_Q_PROPERTY_READ(mouse_pos)
    IMPLEMENT_Q_PROPERTY_READ(world_scale)
    IMPLEMENT_Q_PROPERTY_READ(world_scale_power)
    IMPLEMENT_Q_PROPERTY_READ(category_for_creation)
    IMPLEMENT_Q_PROPERTY_READ(is_creation_mode)
    IMPLEMENT_Q_PROPERTY_READ(fit_to_view_on_load)
    IMPLEMENT_Q_PROPERTY_READ(selected_label)
    IMPLEMENT_Q_PROPERTY_READ(status)
    IMPLEMENT_Q_PROPERTY_READ(is_loading_image)
};
