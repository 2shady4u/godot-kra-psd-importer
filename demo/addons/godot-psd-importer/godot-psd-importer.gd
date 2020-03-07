# ############################################################################ #
# Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
# Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
# Licensed under the MIT License.
# See LICENSE in the project root for license information.
# ############################################################################ #

tool
extends EditorPlugin

var dock = null

func _enter_tree():
	dock = preload("res://addons/godot-psd-importer/editor/ImportDock.tscn").instance()
	dock.connect("exported_textures_created", self, "_on_exported_textures_created")
	add_control_to_dock(DOCK_SLOT_LEFT_UR, dock)

func _exit_tree():
	# Remove from docks (must be called so layout is updated and saved)
	remove_control_from_docks(dock)
	# Remove the node
	dock.free()

func _on_exported_textures_created():
	self.get_editor_interface().get_resource_filesystem().scan_sources()
