extends Node2D

const PSDImporter := preload("res://addons/godot-psd-importer/bin/gdpsdimporter.gdns")

var psd_importer

func _ready():
	psd_importer = PSDImporter.new()

	var result : int = psd_importer.test()
	print(result)
