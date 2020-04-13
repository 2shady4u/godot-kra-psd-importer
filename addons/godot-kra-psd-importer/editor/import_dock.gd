# ############################################################################ #
# Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
# Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
# Licensed under the MIT License.
# See LICENSE in the project root for license information.
# ############################################################################ #

tool
extends Control

const NO_ERROR_TEXT := "No errors..." 

enum FIELDS {RAW_FILE, TARGET_FOLDER}
#enum EXPORT_TYPE {PNG, TGA}

var editor_plugin : EditorPlugin = null

var _editor_file_dialog := EditorFileDialog.new()
var _data_fields := {
	"raw_file_path": "res://addons/godot-kra-psd-importer/examples/PSDExample.psd", 
	"target_folder_path": "res://",
	#"export_type": EXPORT_TYPE.PNG,
	"crop_to_canvas": true,
	"resize_factor": 1,
	"mirror_universe": true
	}
var _is_everything_connected := false
var _verbose_mode := true

const KRAPSDImporter := preload("res://addons/godot-kra-psd-importer/bin/gdkrapsdimporter.gdns")

onready var _raw_file_line_edit : LineEdit
onready var _raw_file_dialog_button : Button
onready var _target_folder_line_edit : LineEdit
onready var _target_folder_dialog_button : Button
#onready var _export_type_option_button : OptionButton
onready var _crop_check_box : CheckBox
onready var _mirror_check_box : CheckBox
onready var _resize_h_slider : HSlider
onready var _resize_line_edit : LineEdit

onready var _import_button : Button

onready var _error_label : Label

onready var _packed_scene_creator : Node

func _process(_delta : float):
	if OS.get_name() != "Windows" and OS.get_name() != "X11":
		push_error("Failed to load plugin for OS '{0}', plugin is only available for Windows and X11!".format([OS.get_name()]))
		set_process(false)
		return

	# Lazy initialization to avoid any problems...
	if not _is_everything_connected:
		if _verbose_mode : print("Attempting to connect all necessary signals!")
		_packed_scene_creator = $PackedSceneCreator
		
		_import_button = $VSplitContainer/BuildVBoxContainer/HBoxContainer/ImportButton
		_import_button.connect("pressed", self, "_import_button_pressed")

		var grid_container := $VSplitContainer/MainVBoxContainer/Panel/MarginContainer/ScrollContainer/VBoxContainer/GridContainer

		var raw_file_container := grid_container.get_node("RawFileContainer")
		_raw_file_line_edit = raw_file_container.get_node("RawFileLineEdit")
		_raw_file_dialog_button = raw_file_container.get_node("RawFileDialogButton")

		_raw_file_line_edit.connect("text_entered", self, "_on_file_or_folder_selected", [FIELDS.RAW_FILE])
		_raw_file_line_edit.text = _data_fields.raw_file_path
		_raw_file_dialog_button.connect("pressed", self, "_on_dialog_button_pressed", [FIELDS.RAW_FILE])
		_raw_file_dialog_button.icon = get_icon("Folder", "EditorIcons")

		var target_folder_container := grid_container.get_node("TargetFolderContainer")
		_target_folder_line_edit = target_folder_container.get_node("TargetFolderLineEdit")
		_target_folder_dialog_button = target_folder_container.find_node("TargetFolderDialogButton")
		_target_folder_dialog_button.icon = get_icon("Folder", "EditorIcons")

		_target_folder_line_edit.connect("text_entered", self, "_on_file_or_folder_selected", [FIELDS.TARGET_FOLDER])
		_target_folder_line_edit.text = _data_fields.target_folder_path
		_target_folder_dialog_button.connect("pressed", self, "_on_dialog_button_pressed", [FIELDS.TARGET_FOLDER])

		#_export_type_option_button = grid_container.get_node("ExportTypeButton")
		#_export_type_option_button.connect("item_selected", self, "_on_item_selected")

		_crop_check_box = grid_container.get_node("CropCheckBox")
		_crop_check_box.pressed = _data_fields.crop_to_canvas
		_crop_check_box.connect("toggled", self, "_on_crop_check_box_toggled")

		_mirror_check_box = grid_container.get_node("MirrorUniverseCheckBox")
		_mirror_check_box.pressed = _data_fields.mirror_universe
		_mirror_check_box.connect("toggled", self, "_on_mirror_check_box_toggled")

		var resize_container := grid_container.get_node("ResizeContainer")
		_resize_h_slider = resize_container.get_node("ResizeHSlider")
		_resize_line_edit = resize_container.get_node("ResizeLineEdit")
		_resize_h_slider.value = _data_fields.resize_factor
		_resize_line_edit.text = String(_data_fields.resize_factor)
		_resize_h_slider.connect("value_changed", self, "_on_resize_h_slider_value_changed")

		_error_label = $VSplitContainer/MainVBoxContainer/ErrorLabel
		_error_label.text = NO_ERROR_TEXT
		_error_label.set("custom_colors/font_color", Color.green)

		add_child(_editor_file_dialog)

		_is_everything_connected = true
		if _verbose_mode:
			print("-> SUCCESFUL INITIALIZATION!")
	else:
		set_process(false)

func _on_dialog_button_pressed(data_field : int):
	if _verbose_mode: print("Dialog button pressed...")
	_editor_file_dialog.current_file = ""
	_editor_file_dialog.clear_filters()

	match data_field:
		FIELDS.RAW_FILE:
			_editor_file_dialog.connect("file_selected", self, "_on_file_or_folder_selected", [data_field])
			_editor_file_dialog.set_mode(FileDialog.MODE_OPEN_FILE)
			_editor_file_dialog.add_filter("*.psd;Photoshop Document")
			_editor_file_dialog.add_filter("*.kra;Photoshop Document")
		FIELDS.TARGET_FOLDER:
			_editor_file_dialog.connect("dir_selected", self, "_on_file_or_folder_selected", [data_field])
			_editor_file_dialog.set_mode(FileDialog.MODE_OPEN_DIR)

	_editor_file_dialog.set_access(FileDialog.ACCESS_FILESYSTEM)
	_editor_file_dialog.popup_centered(Vector2(1280, 800))

func _on_file_or_folder_selected(path: String, data_field : int):
	if _verbose_mode: print("File or folder selected...'{0}'".format([path]))
	match data_field:
		FIELDS.RAW_FILE:
			_data_fields.raw_file_path = ProjectSettings.localize_path(path)
			_raw_file_line_edit.text = _data_fields.raw_file_path
			_raw_file_line_edit.update()
			if _editor_file_dialog.is_connected("file_selected", self, "_on_file_or_folder_selected"):
				_editor_file_dialog.disconnect("file_selected", self, "_on_file_or_folder_selected")
		FIELDS.TARGET_FOLDER:
			_data_fields.target_folder_path = ProjectSettings.localize_path(path)
			_target_folder_line_edit.text = _data_fields.target_folder_path
			_target_folder_line_edit.update()
			if _editor_file_dialog.is_connected("dir_selected", self, "_on_file_or_folder_selected"):
				_editor_file_dialog.disconnect("dir_selected", self, "_on_file_or_folder_selected")

#func _on_item_selected(id : int):
#	if _verbose_mode: print("Item selected...'{0}'".format([id]))
#	_data_fields.export_type = id

func _on_crop_check_box_toggled(button_pressed : bool):
	if _verbose_mode: print("Cropping CheckBox state toggled...'{0}'".format([button_pressed]))
	_data_fields.crop_to_canvas = button_pressed

func _on_mirror_check_box_toggled(button_pressed : bool):
	if _verbose_mode: print("Mirror CheckBox state toggled...'{0}'".format([button_pressed]))
	_data_fields.mirror_universe = button_pressed

func _on_resize_h_slider_value_changed(value : float):
	_data_fields.resize_factor = value
	_resize_line_edit.text = String(value)

func _import_button_pressed() -> void:
	if _verbose_mode: print("Import button pressed...")

	_import_button.disabled = true
	_error_label.text = "Importing... please wait!"
	_error_label.set("custom_colors/font_color", Color.white)
	_error_label.update()
	
	# Make sure it is actually updated!
	yield(get_tree(), "idle_frame")
	yield(VisualServer, "frame_post_draw")
	
	var kra_psd_importer = KRAPSDImporter.new()
	kra_psd_importer.raw_file_path = _data_fields.raw_file_path
	kra_psd_importer.target_folder_path = _data_fields.target_folder_path
	#kra_psd_importer.export_type = _data_fields.export_type
	kra_psd_importer.crop_to_canvas = _data_fields.crop_to_canvas
	kra_psd_importer.resize_factor = _data_fields.resize_factor
	kra_psd_importer.mirror_universe = _data_fields.mirror_universe
	kra_psd_importer.verbose_mode = _verbose_mode

	if _data_fields.mirror_universe:
		kra_psd_importer.connect("texture_created", _packed_scene_creator, "_register_texture_from_importer")
		var structure_name : String = _data_fields.raw_file_path.get_basename().get_file()
		_packed_scene_creator.start_mirorred_layer_structure(structure_name)

	var success : bool = kra_psd_importer.export_all_layers()

	yield(editor_plugin.scan_sources(), "completed")
	if not success:
		_error_label.text = kra_psd_importer.error_message
		_error_label.set("custom_colors/font_color", Color.red)
	else:
		_error_label.text = NO_ERROR_TEXT
		_error_label.set("custom_colors/font_color", Color.green)
	_error_label.update()
	_import_button.disabled = false

	if _data_fields.mirror_universe:
		var result : int = _packed_scene_creator.finish_mirorred_layer_structure()
		if result == OK:
			editor_plugin.open_scene_from_path(_packed_scene_creator.layer_structure_path)
