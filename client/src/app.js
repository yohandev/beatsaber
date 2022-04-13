import { Box3, BoxGeometry, ConeGeometry, Mesh, MeshBasicMaterial, PerspectiveCamera, Scene, Vector3 } from "three";

import level from './level.json';

// Hard-coded level
//  time: Time(in seconds) when this block appears
//  dir: Direction(enum value) of this block
song = new Audio('http://608dev-2.net/sandbox/sc/team27/pkg/song.mp3');
bpm = 96;
notes = level['_notes'].map(n => ({
    time: n['_time'] * bpm / 60,
    col: n['_type'],
    dir: n['_cutDirection'],
    x: n['_lineIndex'],
    y: n['_lineLayer'],
})).filter(n => (n.col <= 1));

// Arbitrary space multiplier between blocks
space = 10;
// Arbitrary distance a user has to press the keys for a block
hit_threshold = 2;

// Maps direction enum value to rotation(euler angles), arrow inputs
directions = {
    0: { angle: 0,   keys: ["ArrowUp"]},  // Up
    1: { angle: 180, keys: ["ArrowDown"]},  // Down
    2: { angle: 90,  keys: ["ArrowLeft"]},  // Left
    3: { angle: 270, keys: ["ArrowRight"]},  // Right
    4: { angle: 45,  keys: ["ArrowUp", "ArrowLeft"]},  // Up left
    5: { angle: 315, keys: ["ArrowUp", "ArrowRight"]},  // Up right
    6: { angle: 135, keys: ["ArrowDown", "ArrowLeft"]},  // Down left
    7: { angle: 225, keys: ["ArrowDown", "ArrowRight"]},  // Down right
    8: { angle: 0,   keys: []},  // Any(dot note)
};

// Game state
state = {};

// Runs once
export const setup = (w, h) => {
    state.scene = new Scene();
    state.cam = new PerspectiveCamera(75, w / h, 0.1, 50);

    // const box = new BoxGeometry(0.5, 0.5, 0.5);
    const box = new ConeGeometry(0.25, 0.5);
    const rmat = new MeshBasicMaterial({ color: 0xff0000 });
    const bmat = new MeshBasicMaterial({ color: 0x0000ff });

    // Hitboxes
    box.computeBoundingBox();
    state.pbox = new Box3(new Vector3(-10, -10, -1), (10, 10, 1));
    state.nbox = box.boundingBox;

    // Initialize level
    for (let n of notes) {
        n.obj = new Mesh(box, n.col ? bmat : rmat);

        n.obj.position.x = n.x;
        n.obj.position.y = n.y;
        n.obj.position.z = -n.time * space;
        n.obj.rotateZ(directions[n.dir].angle * Math.PI / 180);

        state.scene.add(n.obj);
    }
    state.cam.position.x = 1.5;
    state.cam.position.y = 0.5;

    // Start music
    song.play();
    state.time = 0;
};

// Runs every frame(~60fps)
export const loop = (gl, dt, input) => {
    // Timer
    state.time += dt;
    // Calculate collisions
    for (const { obj, dir } of notes) {
        // Positions must be more or less the same
        if (Math.abs(obj.position.z - state.cam.position.z) > hit_threshold) {
            continue;
        }
        // Every key is hit
        if (directions[dir].keys.every(k => input(k))) {
            obj.removeFromParent();
        }
    }

    // Camera advances depending on bpm
    state.cam.position.z -= (bpm / 60) * space * dt;

    // Render
    gl.render(state.scene, state.cam);
};