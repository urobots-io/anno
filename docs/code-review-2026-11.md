# Code review: crashes and memory leaks (November 2026)

Review of the anno C++ sources for memory leaks and crash risks (null pointers,
out-of-range access, endless loops, uncaught exceptions, object lifetimes).
**No fixes have been applied yet.**

- Reviewed state: branch `2026-1`, commit `3bc6063` ("Use Fluent UI icons"), version 1.14.0.0.
  Line numbers refer to that state.
- Build that matters: the Visual Studio project (`ANNO_USE_OPENCV` defined), so the OpenCV/OpenEXR code paths are live.
- ✔ = checked in the code during the review (B7 also with a test program).
  Items without ✔ come from a reviewer's reading, each with code references and a concrete scenario, but were not re-checked.

Suggested order: A and B (crashes from normal use and from project files), then D (leaks),
then C (only if the Evaluate/REST feature is still used), then E. One commit per group.

## A. Crashes reachable through normal use

### A1 ✔ Toolbar Undo/Redo while drawing leaves a null selected marker
- `MainWindow.cpp:451-452` connects the toolbar actions directly to `QUndoStack::undo/redo`.
  Only the Ctrl+Z shortcut goes through `MainWindow::OnUndo` (`MainWindow.cpp:464`), which also calls `ui.desktop->AbortCreation()`.
- Undoing a create command emits `select_label_required({})`; `DesktopWidget::set_selected_label` (`DesktopWidget.cpp:79`)
  sets `selected_label_ = nullptr` but `cursor_mode_` stays `creation_in_progress` / `modifying`.
- Null dereferences: `DesktopWidget.cpp:827` (`selected_label_->OnCreateMove`), `:763` (`OnCreateClick`), `:104` (`CancelExtraAction`), also `:264`, `:585`.
- Scenario: choose a polygon category, click once on the image, click the toolbar Undo button, move the mouse over the image → crash.
- Fix: route the toolbar actions through `OnUndo`/`OnRedo` (or call `AbortCreation()` there); in `set_selected_label`, when `owner` is null,
  switch `creation_in_progress` → `creation_start` and `modifying`/`move_handle*` → `select`; add `if (!selected_label_) break;` guards.

### A2 ✔ Undo after deleting a marker type or category restores orphaned markers
- `FileModel::Delete` (`FileModel.cpp:31-49`) clears a file's undo stack only if that file currently contains affected markers.
  Markers held by `DeleteLabelFileModelCommand` / `DeleteAllLabelsFileModelCommand` in other files survive.
  `LabelCategory` holds only a `weak_ptr` to its definition, so `GetDefinition()` returns null once the type is deleted.
- Same pattern in `UpdateDefinitionSharedLabels`, `ReconnectSharedProperties`, `UpdateDefinitionCustomProperties` (FileModel.cpp ~201-349).
- Scenario: on image A create a marker of type X and delete it; delete marker type X in the Toolbox; select A and press Ctrl+Z
  → `FileModel::GetOwner` dereferences null (`FileModel.cpp:118` `def->is_shared()`); Save crashes at `ApplicationModel.cpp:443`.
  With a deleted category instead: the marker is saved with a missing category id and the project **fails to open** afterwards
  ("has no category", `ApplicationModel.cpp:314`).
- Fix: add `ApplicationModel::ClearAllUndoStacks()` and call it in both `Delete` overloads, `UpdateDefinitionSharedCount`,
  `UpdateDenitionSharedProperties`, `UpdateDefinitionCustomProperties`; add `if (!def) return label;` in `GetOwner`
  and skip markers without definition in `SaveProject`.

### A3 Deleting the category used for drawing creates orphaned markers
- `ToolboxWidget::OnCurrentChanged` returns early on an invalid index (`ToolboxWidget.cpp:93-95`), so `SelectionChanged` is not emitted;
  when a whole type is deleted, Qt emits no `currentChanged` at all. `DesktopWidget` keeps `category_for_creation_`.
- Scenario: select the only category of a type, delete it, click on the image, save → the project fails to load.
- Fix: in `MainWindow::OnDeleteRequest`, before `model_.Delete(...)`: `ui.desktop->set_is_creation_mode(false); ui.definition_editor->Select({}, {});`
  plus A2. Related: after a project switch, `LabelDefinitionPropertiesWidget` still edits the old project's type;
  connect `label_definitions_changed` to `definition_editor->Select({}, {})`.

### A4 ✔ Polyline / tool markers with 0 points
- `PolylineLabel.cpp:131`: `for (size_t i = 0; i < handles_.size() - 1; ++i)` — `size()-1` wraps around on an empty vector.
  Reached from right-click in select mode (`DesktopWidget::StartExtraActionUnderCursor` → `DetectExtraAction`).
- Empty polylines come from `"value": ""` in a file, or from a polyline/tool type with "stamp" enabled (the stamp clone has no handles).
- Also `handles_.front()` without checks in `GetTransform` of PointLabel.cpp:32, PolygonLabel.cpp:440, PolylineLabel.cpp:155
  (reached from rendering scripts calling `SetBaseTransform`), and `&points[0]` on empty vectors in PolygonLabel.cpp:105/109, PolylineLabel.cpp:44.
- Fix: `for (size_t i = 0; i + 1 < handles_.size(); ++i)`; `if (handles_.empty()) return QTransform();`; `if (points.empty()) return;`.

### A5 ✔ Paste marker type with no project open
- `ToolboxWidget.cpp:360` `definitions_->InsertNewDefinition(...)` without null check; the Paste button is never disabled.
- Scenario: copy a marker type, File → New Project, cancel the folder dialog, click Paste.
- Fix: `if (!definitions_) return;` and enable/disable Paste (and Copy/Properties) in `SetDefinitionsModel`.

### A6 ✔ Quitting while an image is loading aborts the process
- `DesktopWidget.cpp:144` creates `ImageLoader` threads with the widget as parent; `~DesktopWidget` (line 47) never waits.
  Qt calls `qFatal("QThread: Destroyed while thread is still running")`.
- Scenario: REST dataset or a large EXR, select an image and quit while "Loading…" is shown.
- Fix: `DesktopWidget::~DesktopWidget() { for (auto l : loaders_) { l->disconnect(this); l->wait(); } }`

## B. Crashes and hangs from malformed or unusual files

### B7 ✔ (tested) Marker value parser loops forever on non-numeric input
- `Label.cpp:74-81`: `while (!stream.atEnd()) { stream >> x >> y; handles.push_back(...); }`.
  When `QTextStream >> float` fails it does not consume the bad character, so `atEnd()` never becomes true and a handle is added every iteration
  → hang and out-of-memory. Test results: `"10,20 30,40"` and `"1 2 x"` never terminate; `"1 2 "` (trailing space) yields a phantom second point.
- Affects point, polyline, tool, polygon (and its child contours), rect.
- Fix: read in a loop that stops on `stream.status() != QTextStream::Ok` and skips non-finite values.

### B8 Point marker with empty value
- `PointLabel` has no `FromStringsList`; with `"value": ""` it has 0 handles and `PointLabel.cpp:22` (`handles_.front()` in OnPaint),
  `:16`, `:38` (CenterTo/MoveBy), `:32` (GetTransform) read an empty vector. Evaluate import (`ApplicationModel.cpp:947-948`) also hits it.
- Fix: override `FromStringsList` and add a (0,0) handle when none was read.

### B9 ✔ Negative `shared_index`
- `ApplicationModel.cpp:321-331` checks only the upper bound; `"shared_index": -1` reads `shared_labels[-1]` and `ProxyLabel` dereferences garbage.
- Fix: `if (definition->is_shared() && (shared_index < 0 || shared_index >= int(definition->shared_labels.size())))`.

### B10 ✔ `geometry::WrapAngle` endless loop
- `geometry.h:7-8` loops `while (src > M_PI) src -= 2*M_PI;` — never ends for `inf` or |angle| above ~1e17.
  Angles from files (`"inf"`, `"1e30"`) reach it via `ToStringsList` (save, drag start, clone) in OrientedPoint/OrientedCircle/OrientedRect labels.
- Fix: `if (!std::isfinite(a)) return 0; return std::remainder(a, 2 * M_PI);`

### B11 Huge `shared_count`
- `Serialization.cpp:58-59` / `:136` create `shared_count` labels without a limit; `"shared_count": 2000000000` → out of memory.
- Fix: clamp (e.g. 0..1000) and report an error.

### B12 ✔ JSON error message reads past the buffer
- `qjson_helpers.cpp:47-48` converts `error.offset + 80` bytes without bounding to `size()`; a truncated or empty file reads up to 80 bytes past the end.
  `ApplicationModel.cpp:108-110` does not check `jsonFile.open()`.
- Fix: bound offset and length to the buffer size; `if (!jsonFile.open(QFile::ReadOnly)) { errors << ...; return false; }`.

### B13 ✔ Damaged EXR image terminates the app
- `ImageLoader.cpp:76-98`: the OpenEXR header is parsed even when `imdecode` failed, and OpenEXR exceptions escape `QThread::run` → `std::terminate`.
- `MemStream::read` (`ImageLoader.cpp:42-53`): `pos_ + n < size` should be `<=` (last byte unreadable), returns false instead of throwing on short reads, `pos_` is `int`.
- Fix: skip header parsing when `!image_ok`; wrap it in `try { ... } catch (const std::exception& e) { error_text_ = e.what(); }`;
  fix `read` (throw `Iex::InputExc` on EOF), make `pos_` `Imf::Int64`.

### B14 ✔ 3D view: point cloud and depth images
- `PointCloudDisplayWidget.cpp:589-597`: `while (!cin.eof()) { cin >> x >> y >> z >> color >> intencity; ... }` loops forever on a binary PCD
  or any unexpected token; `:604` `&vertices[0]` on an empty vector.
- `:424` `depth.at<unsigned short>(y, x)` and `:457` `xyz.at<cv::Vec3w>(y, x)` iterate over the RGB size; a smaller depth/XYZ image reads out of bounds.
- Fix: `while (cin >> x >> y >> z >> color >> intencity)`, empty checks before `allocate`, ignore depth/XYZ images whose size differs from the RGB image.

## C. Evaluate feature (REST server)

### C15 ✔ Image buffer size assumes 4 channels
- `ApplicationModel.cpp:866`: `QByteArray data((char*)(image.data), image.rows * image.cols * 4);` but images are loaded with
  `IMREAD_ANYCOLOR | IMREAD_ANYDEPTH` (`ImageLoader.cpp:78`) and passed unconverted (`MainWindow.cpp:537/540`) → over-read for gray/BGR images.
  The ROI crop is a non-contiguous `cv::Mat`, so its data is wrong too.
- Fix: convert to 8-bit BGRA, `clone()` if not continuous, size from `total() * elemSize()`.

### C16 ✔ Server reply with an unknown category
- `ApplicationModel.cpp:896-900` finds the definition by type name without checking the category; `:936`
  `label->SetCategory(definition->GetCategory(category))` sets null → crash on paint/save (`ApplicationModel.cpp:442`).
  For shared definitions a plain label is created instead of a `ProxyLabel`.
- Fix: check the category (`if (!cat) { errors << ...; continue; }`), reject or proxy shared definitions.

### C17 Nested event loop without timeout
- `rest.cpp:31-33`, `:68-70`: `QEventLoop loop; ... loop.exec();` — user input is processed meanwhile and there is no timeout.
  Closing the project during Evaluate → `get_label_definitions()` null at `ApplicationModel.cpp:889`; a server that never answers hangs the app.
  Remote save marks edits made during the upload as saved.
- Fix: `request.setTransferTimeout(30000)`, `loop.exec(QEventLoop::ExcludeUserInputEvents)`, re-check `label_definitions_` after the call.

### C18 ROI crop with huge or NaN rectangle
- `ImageModelOpenCV.cpp:141` `cv::Mat::zeros(roi.size(), ...)` with sizes from the rect marker; huge → `cv::Exception` (out of memory), NaN → negative size assert;
  thrown inside the `MainWindow::OnEvaluateInROI` slot.
- Fix: crop to the intersection with the image; reject non-finite rects; `try/catch (cv::Exception&)`.

## D. Memory leaks

### D19 ✔ Deleted file/folder tree element is never freed
- `SourcePicturesTreeModel.cpp:412` erases the element from `children` without deleting it (whole subtree for folders, with its `shared_ptr<FileModel>`s).
- Fix: `RecursiveDelete(file);` after `endRemoveRows()`.

### D20 ✔ New item delegate on every marker click
- `CustomPropertyTableItemDelegate.cpp:18` creates a new delegate each call (from `LabelPropertiesWidget.cpp:95`); the old one is never deleted.
- Fix: create it only when `!qobject_cast<CustomPropertyTableItemDelegate*>(table_view->itemDelegate())`.
  On Qt 5, `LabelPropertiesWidget.cpp:70-71` (`delete model(); setModel(nullptr);`) may also leak a selection model; keep one model and swap its label.

### D21 Delegates without parent in the Label Definition dialog
- `LabelDefinitionPropertiesDialog.cpp:37-38`: `new SharedPropertiesTableDelegate()`, `new CustomPropertiesEditorTableItemDelegate()` — leak per dialog open.
- Fix: add a `QObject* parent` constructor and pass `this`.

### D22 ✔ Errors dialog model without parent
- `ErrorsListDialog.cpp:21`: `new QStandardItemModel(errors.size(), 1)` — leaks on every error dialog.
- Fix: pass `this` as parent.

## E. Data bugs (not crashes)

- **E23 ✔** `FileModel.cpp:293`: missing `break` after `default: result = value;` in `ConvertVariant` — custom properties with an unknown type are converted to int.
- **E24 ✔** `OrientedCircleLabel.cpp:48`: `radius_.set(def->GetSharedPropertyValue(ANGLE, angle));` should be `angle_.set(...)` (stamp angle overwrites the radius).
- **E25** `ApplicationModel.cpp:716-737`: changing `shared_count` from N>0 to M>0 creates new shared instances while existing proxies keep the old ones;
  reuse existing clients for indices below the old count.
- **E26** `FileModel.cpp:65`, `:84-90`: undo-stack "top" checks use `count()-1` instead of `index()-1`; after an Undo, a text edit can go unrecorded and Redo applies stale text.
- **E27** Smaller items:
  - Deleting a folder that contains the open image keeps the image open; `OnNewFileSystem` should always `emit FileModelSelected({})`, and
    `OnDeleteFile`/`OnReloadFolder` should emit it when the current index becomes invalid.
  - `ClearProject` does not reset `project_script_`, so a new project inherits the previous script.
  - `ToolLabel` does not override `Clone()`, so a stamped tool becomes a plain polyline.
  - `PolygonLabel::Triangulate` (PolygonLabel.cpp:364-381) checks all segment pairs on every handle move — UI freezes on polygons with 10k+ points.
  - `LocalFilesystem::CreateSubfolder` builds `destination_abs + name` without `/`.
  - `SourcePicturesWidget.cpp:354` `static_cast<SourcePicturesTreeModel*>` on what is actually the proxy model (works by accident).
  - Qt (non-OpenCV) build: a failed image decode leaves `error_text_` empty.

## Checked and found safe

- `LabelDefinitionsTreeModel` with both a Qt parent and `shared_ptr` owners: `~MainWindow` calls `ClearProject()` first (fragile, not reachable today).
- No `shared_ptr` cycles between labels and handles or proxies and clients; `LabelHandle::parent_` is cleared on delete.
- `Clone()` results are owned immediately; `ScriptPainter` on the stack has a parent, so the JS engine does not own it.
- Deleting an `ImageLoader` in its own `finished` slot is fine.
- QUndoCommand ownership, reply/multipart lifetimes in `rest.cpp`, `categories[0]` uses, `LabelFactory`, PropertyDatabase, settings,
  tree-model `internalPointer` casts, `ColorTransformer` table indices, `GetBackgroundPixelValues` bounds, earcut indices,
  Desktop3dWindow lifetime, menu/`QAction` parenting, `NavigationModel` indices, `StartupDialog` recent-project filtering.
