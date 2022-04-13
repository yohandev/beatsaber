import { WebGLRenderer } from 'three';
import { setup, loop } from './app';

const w = window.innerWidth;
const h = window.innerHeight;
const b = document.body;

const gl = new WebGLRenderer();

gl.setSize(w, h);
b.appendChild(gl.domElement);

let last = 0;
const input = (() => {
    const state = {};

    // https://stackoverflow.com/a/48750898
    window.addEventListener('keyup', e => state[e.key] = false);
    window.addEventListener('keydown', e => state[e.key] = true);

    return k => state.hasOwnProperty(k) && state[k] || false;
})();

function frame(time) {
    requestAnimationFrame(frame);
    loop(gl, (time - last) / 1000, input);

    last = time;
}
document.body.onclick = () => {
    setup(w, h);
    frame(last = 0);
}