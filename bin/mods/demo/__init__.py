info = {
    "name": "Example",
    "author": "Grady O'Connell",
    "email": "flipcoder@gmail.com"
}

import qor
import time

class State:
    def __init__(self):
         
        
        self.players = [
            qor.Sprite("data/actors/actor.json", "maskKid"),
            qor.Sprite("data/actors/actor.json", "fetusMaximus"),
            qor.Sprite("data/actors/actor.json", "grampire")
        ]

        self.players[0].position = [-16, 0, -16]
        self.players[1].position = [0, 0, -32]
        self.players[2].position = [16, 0, -48]

        for player in self.players:
            player.states([
                player.state_id("stand"),
                player.state_id("down")
            ])
            player.state(player.state_id("walk"))
            player.spawn()
            
        self.player = qor.Player3D(qor.camera())

def preload():
    global state
    #time.sleep(2)
    state = State()

def unload():
    global state

def logic(t):
    global state
    qor.bg_color([0,0,0])
    qor.ortho(False)

def render():
    global state

