extends Node

var socket: WebSocketPeer = null
var others = {}

var player = null

func _ready():
	player = get_node_or_null("../Player")
	if player == null:
		printerr("No player found")
		get_tree().quit()
	connect_to_server("127.0.0.1", 9002)
	
func _process(_delta):
	socket.poll()
	
	var socket_state = socket.get_ready_state()

	if socket_state == WebSocketPeer.STATE_CLOSED || socket_state == WebSocketPeer.STATE_CLOSING:
		printerr("Socket closed")
		get_tree().quit()
		
	
	while socket.get_available_packet_count():
		var packet_str = socket.get_packet().get_string_from_utf8()
		if packet_str.is_empty():
			print("Bad packet")
			get_tree().quit()
		var variant = JSON.parse_string(packet_str)
		print(variant)
		if variant.has("PlayerState"):
			var coords = variant["PlayerState"]["position"]
			player.position = Vector3(coords[0], coords[1], coords[2])
			#set_position(Vector3(coords[0], coords[1], coords[2]))
		elif variant.has("OthersState"):
			var others_update = variant["OthersState"] as Array
			for other_player in others_update:
				var id = int(other_player["id"])
				others[id].velocity = Vector3(other_player["position"][0], other_player["position"][1], other_player["position"][2])

func connect_to_server(host: String, port: int):
	socket = WebSocketPeer.new()

	var url: String
	if OS.is_debug_build():
		url = "ws://"
	else:
		url = "wss://"
	url += "%s:%s" % [host, port]

	print("Connecting to %s" % url)
	if socket.connect_to_url(url, null) != OK:
		printerr("Could not connect")
		get_tree().quit()
