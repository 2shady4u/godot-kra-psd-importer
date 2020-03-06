# ############################################################################ #
# Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
# Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
# Licensed under the MIT License.
# ############################################################################ #

tool
extends Control

enum FIELDS {PSD_FILE, TARGET_FOLDER}
enum EXPORT_TYPE {PNG, TGA}

var _editor_file_dialog := EditorFileDialog.new()
var _data_fields := {
	"psd_file_path": "res://addons/godot-psd-importer/examples/Sample.psd", 
	"target_folder_path": "res://",
	"export_type": EXPORT_TYPE.PNG
	}
var _is_everything_connected := false
var _verbose_mode := true

const PSDImporter := preload("res://addons/godot-psd-importer/bin/gdpsd.gdns")

onready var _import_button : Button

onready var _psd_file_line_edit : LineEdit
onready var _psd_file_dialog_button : Button

onready var _target_folder_line_edit : LineEdit
onready var _target_folder_dialog_button : Button

onready var _export_type_option_button : OptionButton

signal exported_textures_created

func _process(_delta : float):
	if OS.get_name() != "Windows":
		push_error("Failed to load plugin for OS '{0}', plugin is only available for Windows!".format([OS.get_name()]))
		set_process(false)
		return

	# Lazy initialization to avoid any problems...
	if not _is_everything_connected:
		if _verbose_mode : print("Attempting to connect all necessary signals!")
		_import_button = $VSplitContainer/BuildVBoxContainer/HBoxContainer/ImportButton
		_import_button.connect("pressed", self, "_import_button_pressed")

		var grid_container := $VSplitContainer/MainVBoxContainer/Panel/MarginContainer/ScrollContainer/VBoxContainer/GridContainer

		var psd_file_container := grid_container.get_node("PsdFileContainer")
		_psd_file_line_edit = psd_file_container.get_node("PsdFileLineEdit")
		_psd_file_dialog_button = psd_file_container.get_node("PsdFileDialogButton")

		_psd_file_line_edit.connect("text_entered", self, "_on_file_or_folder_selected", [FIELDS.PSD_FILE])
		_psd_file_line_edit.text = _data_fields.psd_file_path
		_psd_file_dialog_button.connect("pressed", self, "_on_dialog_button_pressed", [FIELDS.PSD_FILE])

		var target_folder_container := grid_container.get_node("TargetFolderContainer")
		_target_folder_line_edit = target_folder_container.get_node("TargetFolderLineEdit")
		_target_folder_dialog_button = target_folder_container.find_node("TargetFolderDialogButton")

		_target_folder_line_edit.connect("text_entered", self, "_on_file_or_folder_selected", [FIELDS.TARGET_FOLDER])
		_target_folder_line_edit.text = _data_fields.target_folder_path
		_target_folder_dialog_button.connect("pressed", self, "_on_dialog_button_pressed", [FIELDS.TARGET_FOLDER])

		_export_type_option_button = grid_container.get_node("ExportTypeButton")

		_export_type_option_button.add_item("PNG", EXPORT_TYPE.PNG)
		_export_type_option_button.add_item("TGA", EXPORT_TYPE.TGA)
		
		_export_type_option_button.connect("item_selected", self, "_on_item_selected")
	
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
		FIELDS.PSD_FILE:
			_editor_file_dialog.connect("file_selected", self, "_on_file_or_folder_selected", [data_field])
			_editor_file_dialog.set_mode(FileDialog.MODE_OPEN_FILE)
			_editor_file_dialog.add_filter("*.psd;Photoshop Document")
		FIELDS.TARGET_FOLDER:
			_editor_file_dialog.connect("dir_selected", self, "_on_file_or_folder_selected", [data_field])
			_editor_file_dialog.set_mode(FileDialog.MODE_OPEN_DIR)

	_editor_file_dialog.set_access(FileDialog.ACCESS_FILESYSTEM)
	_editor_file_dialog.popup_centered(Vector2(1280, 800))

func _on_file_or_folder_selected(path: String, data_field : int):
	if _verbose_mode: print("File or folder selected...'{0}'".format([path]))
	match data_field:
		FIELDS.PSD_FILE:
			_data_fields.psd_file_path = ProjectSettings.localize_path(path)
			_psd_file_line_edit.text = _data_fields.psd_file_path
			_psd_file_line_edit.update()
			if _editor_file_dialog.is_connected("file_selected", self, "_on_file_or_folder_selected"):
				_editor_file_dialog.disconnect("file_selected", self, "_on_file_or_folder_selected")
		FIELDS.TARGET_FOLDER:
			_data_fields.target_folder_path = ProjectSettings.localize_path(path)
			_target_folder_line_edit.text = _data_fields.target_folder_path
			_target_folder_line_edit.update()
			if _editor_file_dialog.is_connected("dir_selected", self, "_on_file_or_folder_selected"):
				_editor_file_dialog.disconnect("dir_selected", self, "_on_file_or_folder_selected")

func _on_item_selected(id : int):
	if _verbose_mode: print("Item selected...'{0}'".format([id]))
	_data_fields.export_type = id

func _import_button_pressed():
	if _verbose_mode: print("Import button pressed...")
	var psd_importer = PSDImporter.new()
	psd_importer.psd_file_path = _data_fields.psd_file_path
	psd_importer.target_folder_path = _data_fields.target_folder_path
	psd_importer.export_type = _data_fields.export_type

	var result : int = psd_importer.export_psd()
	print(result)

	emit_signal("exported_textures_created")
