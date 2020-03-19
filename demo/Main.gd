extends Node2D

const KRAPSDImporter := preload("res://addons/godot-kra-psd-importer/bin/gdkrapsdimporter.gdns")

var kra_psd_importer

func _ready():
	kra_psd_importer = KRAPSDImporter.new()

	var result : int = kra_psd_importer.test()
	print(result)
