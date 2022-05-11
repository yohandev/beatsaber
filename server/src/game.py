from zipfile import ZipFile
from io import BytesIO

import requests
import base64
import os
import sqlite3

db = '/var/jail/home/team27/users_and_scores.db'
PKG = '/var/jail/home/team27/game.html'

def request_handler(req):
    assert req['method'] == 'GET'
    assert 'id' in req['values']

    id = req['values']['id']
    user = req['values']['user']

    if id in (24, 512, 300, 150):
        set_song_data(user, id)

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


def create_database():
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS game_db
        (user text, score int, highscore int, played_24 int, played_512 int, played_300 int, played_150 int);""")  # noqa: E501

    conn.commit()
    conn.close()


def set_song_data(user, song_id):
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute('''SELECT user, score, highscore, played_24, played_512, played_300, played_150 FROM game_db WHERE user = ?''', (user,))   # noqa: E501
    result = c.fetchone()

    songs_played = {'24': result[3], "512": result[4], "300": result[5], "150": result[6]}
    if songs_played[song_id] == 1:
        conn.commit()
        conn.close()
    else:
        songs_played[song_id] = 1
        c.execute('''UPDATE game_db SET played_24 = (?), played_512 = (?), played_300 = (?), played_150 = (?)  WHERE user = (?)''',
            (songs_played['24'], songs_played['512'], songs_played['300'], songs_played['150'], user))
        conn.commit()
        conn.close()
    return
