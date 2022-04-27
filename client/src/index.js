import { WebGLRenderer } from 'three'
import { BeatSaber } from './game'

const w = window.innerWidth
const h = window.innerHeight
const b = document.body

const gl = new WebGLRenderer()
const game = new BeatSaber()

gl.setSize(w, h)
b.appendChild(gl.domElement)

let last = 0;
const input = (() => {
    const state = {}

    // https://stackoverflow.com/a/48750898
    window.addEventListener('keyup', e => state[e.key] = false)
    window.addEventListener('keydown', e => state[e.key] = true)

    return k => state.hasOwnProperty(k) && state[k] || false
})();

function frame(time) {
    requestAnimationFrame(frame)
    
    if (!last) {
        last = time
    }
    game.loop((time - last) / 1000)
    game.draw(gl)

    last = time;
}

// let btn = document.getElementsByClassName('start-button')
// for (let i = 0; i < btn.length; i++) {
//     btn.item(i).addEventListener('click', () => {
//         game.setup(w, h)
//         frame(last = null)
//         console.log('STARTT')
//     })
// }
document.body.onclick = () => {
    game.setup(w, h)
    frame(last = null)
}