# ############################################################################ #
# Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
# Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
# Licensed under the MIT License.
# See LICENSE in the project root for license information.
# ############################################################################ #

extends Node2D

const KRAPSDImporter := preload("res://addons/godot-kra-psd-importer/bin/gdkrapsdimporter.gdns")

const KRA_RAW_FILE = "res://addons/godot-kra-psd-importer/examples/KRAEXample.kra"
const PSD_RAW_FILE = "res://addons/godot-kra-psd-importer/examples/PSDEXample.psd"
const TARGET_FOLDER = "res://graphics/"

func _ready():
	print("- TESTING KRA EXPORT -")
	var success := test_kra();
	if success:
		print("-> KRA Export was succesful!")
	else:
		print("-> (ERROR) KRA Export returned an error!")
	
	print("- TESTING PSD EXPORT -")
	success = test_psd();
	if success:
		print("-> PSD Export was succesful!")
	else:
		print("-> (ERROR) PSD Export returned an error!")

func test_kra() -> bool:
	var kra_psd_importer = KRAPSDImporter.new()
	kra_psd_importer.raw_file_path = KRA_RAW_FILE
	kra_psd_importer.target_folder_path = TARGET_FOLDER
	kra_psd_importer.export_type = 0 #PNG = 0 & TGA = 1
	kra_psd_importer.crop_to_canvas = false
	kra_psd_importer.resize_factor = 1.0
	kra_psd_importer.mirror_universe = false
	kra_psd_importer.verbose_mode = true
	
	return kra_psd_importer.export_all_layers()
	
func test_psd() -> bool:
	var kra_psd_importer = KRAPSDImporter.new()
	kra_psd_importer.raw_file_path = PSD_RAW_FILE
	kra_psd_importer.target_folder_path = TARGET_FOLDER
	kra_psd_importer.export_type = 0 #PNG = 0 & TGA = 1
	kra_psd_importer.crop_to_canvas = false
	kra_psd_importer.resize_factor = 1.0
	kra_psd_importer.mirror_universe = false
	kra_psd_importer.verbose_mode = true
	
	return kra_psd_importer.export_all_layers()
