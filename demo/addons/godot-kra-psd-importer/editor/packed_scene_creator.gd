# ############################################################################ #
# Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
# Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
# Licensed under the MIT License.
# See LICENSE in the project root for license information.
# ############################################################################ #

tool
extends Node

var layer_structure_path := ""

var _textures_array := []
var _mirrored_layer_structure : Node2D = null

var _is_creating_packed_scene : bool = false

func start_mirorred_layer_structure(structure_name : String):
	_mirrored_layer_structure = Node2D.new()
	_mirrored_layer_structure.name = structure_name
	_is_creating_packed_scene = true
	print("(PackedSceneCreator) Starting mirrored layer structure with name '{0}'.".format([structure_name]))
	_textures_array = []

func _register_texture_from_importer(texture_properties : Dictionary):
	if _is_creating_packed_scene and texture_properties.has("path"):
		print("(PackedSceneCreator) Registering texture with path '{0}'.".format([texture_properties.path]))
		_textures_array.push_back(texture_properties)
	else:
		print("PackedSceneCreator WARNING: Texture did not have path...ignoring call!")

func finish_mirorred_layer_structure() -> int:
	if _is_creating_packed_scene:
		for texture in _textures_array:
			var dir := Directory.new()
			var path = texture.path
			if not dir.file_exists(path):
				push_error("PackedSceneCreator Error: Texture path '{0}' did not point to an existing file!".format([path]))
				continue
			var sprite := Sprite.new()
			sprite.texture = load(path)
			
			sprite.position = texture.get("position", Vector2.ZERO)
			sprite.position = sprite.position + sprite.texture.get_size()/2
			sprite.name = path.get_basename().get_file()

			_mirrored_layer_structure.add_child(sprite)
			sprite.owner = _mirrored_layer_structure

		var scene = PackedScene.new()
		var result = scene.pack(_mirrored_layer_structure)
		if result == OK:
			#var dir := Directory.new()
			layer_structure_path = "res://{0}.tscn".format([_mirrored_layer_structure.name])
			#dir.remove(layer_structure_path)
			ResourceSaver.save(layer_structure_path, scene)
			print("(PackedSceneCreator) Finishing mirrored layer structure. (saved at '{0}')".format([layer_structure_path]))
		else:
			push_error("PackedSceneCreator Error: Packing the mirrored layer structure failed!")
		_is_creating_packed_scene = false
		_textures_array = []
		return result
	else:
		push_error("PackedSceneCreator Error: Could not finish mirrored layer structure, not in progress!")
		return ERR_UNAVAILABLE

