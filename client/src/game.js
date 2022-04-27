import { Mesh, MeshBasicMaterial, PerspectiveCamera, Scene, SphereGeometry } from 'three'
import { BlendFunction, BloomEffect, EffectComposer, EffectPass, KernelSize, RenderPass } from 'postprocessing'
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js'

import lvl from './level.json'
import note_mesh from '../assets/note.obj'
import sword_mesh from '../assets/sword.obj'

/**
 * Game instance
 */
export class BeatSaber {
    map = null      // Level to play
    scene = null    // Three.js scene
    cam = null      // Three.js camera
    composer = null // Three.js post-processing
    
    constructor() {
    }

    /**
     * Called when the game begins
     */
    setup(w, h) {
        this.map = new Map(lvl['_notes'], 140)
        this.scene = new Scene()
        this.cam = new PerspectiveCamera(75, w / h, 0.1, 50)

        this.cam.position.x = 1.5 * Note.size
        this.cam.position.y = 1.5 * Note.size
    }

    /**
     * Called every frame
     */
    loop(dt) {
        this.map.step(this.scene, dt)
    }

    /**
     * Called right after `loop`
     */
    draw(gl) {
        if (!this.composer) {
            this.composer = new EffectComposer(gl)
            this.composer.addPass(new RenderPass(this.scene, this.cam))
            // this.composer.addPass(new EffectPass(this.cam, new BloomEffect({
            //     blendFunction: BlendFunction.SCREEN,
            //     kernelSize: KernelSize.MEDIUM,
            //     luminanceThreshold: 0.4,
            //     luminanceSmoothing: 0.1,
            //     height: 480
            // })))
        }
        this.composer.render()
    }
}

/**
 * A single level of beat saber. Manages events sequentially
 */
export class Map {
    static peek = 15 // Number of beats ahead for when notes should appear
    static anim = 1 // Fade-in animation for notes, in beats

    level = []      // `level.json`["_notes"]
    notes = []      // Notes currently displayed
    last = -1       // Index of last note spawned in level
    bpm = -1        // Beats per minute of level
    now = 0         // Current beat of level(timer)
    lsaber = null   // Left light saber
    rsaber = null   // Right light saber 

    constructor(level, bpm) {
        this.level = level
        this.bpm = bpm
    }

    /**
     * Step the level by (d)elta (t)ime, given the thress.js `scene`
     */
    step(scene, dt) {
        // Load light saber models
        if (!this.lsaber) {
            this.lsaber = new OBJLoader().parse(sword_mesh)
            this.lsaber.children[1].material = new MeshBasicMaterial({ color: 0x111111 })
            this.rsaber = this.lsaber.clone(true)

            this.lsaber.children[0].material = new MeshBasicMaterial({ color: 0xff0000 })
            this.rsaber.children[0].material = new MeshBasicMaterial({ color: 0x0000ff })

            this.lsaber.position.x = 0.5 * Note.size
            this.rsaber.position.x = 2.5 * Note.size
            this.lsaber.position.z = -5
            this.rsaber.position.z = -5

            scene.add(this.lsaber)
            scene.add(this.rsaber)

            setInterval(() => {
                // Move lightsabers
                let d = parseInt(this.query_direction())
                console.log(d)
                this.lsaber.rotation.z = Note.rot[d] * (Math.PI / 180)
            }, 100)
        }

        // Time(s) -> time(beats)
        this.now += (this.bpm / 60) * dt

        // Spawn-in new notes
        while (this.last + 1 < this.level.length && this.level[this.last + 1]['_time'] <= this.now + Map.peek) {
            this.notes.push(new Note(this.level[this.last++]))
        }

        // Step each note
        for (const note of this.notes) {
            note.step(scene, this.now)
        }

        // Destroy old notes
        this.notes = this.notes.filter(n => {
            // Threshold of one beat behind player
            if (this.now - n.beat < 1) {
                return true
            }
            scene.remove(n.obj)
            return false
        })
    }

    query_direction() {
        let xmlHttpReq = new XMLHttpRequest();
        xmlHttpReq.open("GET", "https://608dev-2.net/sandbox/sc/team27/web_server.py?js=true", false); 
        xmlHttpReq.send(null);
        return xmlHttpReq.responseText;
    }
}

/**
 * A single note/block
 */
export class Note {
    static size = 2 // Size of one side, in world space, of a note/block
    static prefabs  // Prefab meshes, key is type(0-3| red | blue | unused | bomb)
        = null
    static dist = 15// Arbitrary distance between notes on z/time axis
    static rot = {  // Rotations
        0: 0,
        1: 180,
        2: 90,
        3: 270,
        4: 45,
        5: 315,
        6: 135,
        7: 225,
        8: 0,
    }

    beat = -1       // The beat on which the note reaches the player
    type = -1       // Type(0-3| red | blue | unused | bomb)
    col = -1        // Column/vertical position(0-2| bottom -> top)
    row = -1        // Row/horizontal position(0-3| left -> right)
    dir = -1        // Cut direction(0-8| ↑ | ↓ | ← | → | ↖ | ↗ | ↙ | ↘ | • |)
    obj = null      // Three.js object in scene

    /**
     * Create a new note given its entry in `level.json`
     */
    constructor(json) {
        this.beat = json['_time']
        this.type = json['_type']
        this.col  = json['_lineLayer']
        this.row  = json['_lineIndex']
        this.dir  = json['_cutDirection']

        // Create prefabs
        if (!Note.prefabs) {
            let note = new OBJLoader().parse(note_mesh)
            // Give arrow bloom material
            note.children[1].material = new MeshBasicMaterial({ color: 0xffffff })

            let bomb = new SphereGeometry(Note.size)

            Note.prefabs = {
                0: note.clone(true),
                1: note.clone(true),
                2: note,
                3: new Mesh(bomb),
            }
            // Red/blue arrow color
            Note.prefabs[0].children[0].material = new MeshBasicMaterial({ color: 0xff0000 })
            Note.prefabs[1].children[0].material = new MeshBasicMaterial({ color: 0x0000ff })
        }
    }

    /**
     * Update the position of this note, given the three.js `scene`
     * and current world time, `now`(in beats)
     * @param {Scene} scene 
     * @param {*} dt 
     */
    step(scene, now) {
        // Instantiate in scene
        if (!this.obj) {
            this.obj = Note.prefabs[this.type].clone(true)
            // this.obj.rotation.z = (Math.PI / 180) * Note.rot[this.dir]
            // this.obj.rotation.y = -Math.PI / 2
            this.obj.rotateZ((Math.PI / 180) * Note.rot[this.dir])
            this.obj.rotateY(-Math.PI / 2)

            scene.add(this.obj)
        }
        // Move
        // this.obj.position = new Vector3(this.row, this.col, this.beat - now).multiplyScalar(Note.size)
        
        this.obj.position.x = this.row * Note.size
        this.obj.position.y = this.col * Note.size
        this.obj.position.z = -(this.beat - now) * Note.size * Note.dist
        // // Fade-in animation
        // let lerp = 1 - (now - this.beat + Map.peek - Map.anim) / Map.anim
        // if (lerp <= 1) {
        //     t
        // }
    }
}