extends Node3D

func _ready():
	set_process(false)
	set_process_input(false)
	set_physics_process(false)

func _input(event):
	print(event.as_text())

func _physics_process(delta):
	if Input.is_action_pressed("move_right"):
		# Move as long as the key/button is pressed.
		#transform.position.x += speed * delta
		print("move_right")
