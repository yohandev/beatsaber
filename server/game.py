from zipfile import ZipFile
from io import BytesIO

import requests
import base64
import os

PKG = '/var/jail/home/team27/game.html'

def request_handler(req):
    assert req['method'] == 'GET'
    assert 'id' in req['values']
    
    id = int(req['values']['id'])

    meta = requests.get(f"https://api.beatsaver.com/maps/id/{id}").json()
    url = meta['versions'][0]['downloadURL']
    lvl = requests.get(url)

    tmp = '/var/jail/home/team27/tmp'
    with ZipFile(BytesIO(lvl.content), 'r') as zip:
        zip.extractall(tmp)

    files = [os.path.join(tmp, f) for f in os.listdir(tmp)]

    info = next(f for f in files if 'info.dat' in f.lower())
    difficulty = next(f for f in files if '.dat' in f and f != info)
    song = next(f for f in files if '.egg' in f or '.ogg' in f)

    with open(info, 'r') as f: info = f.read()
    with open(difficulty, 'r') as f: difficulty = f.read()
    with open(song, 'rb') as f: song = base64.b64encode(f.read()).decode('UTF-8')
    with open(PKG) as f: html = f.read()

    for f in files:
        os.remove(f)
    os.rmdir(tmp)

    return html.replace('<info.dat>', info).replace('<difficulty.dat>', difficulty).replace('<song.egg>', song)
