import sqlite3

db = '/var/jail/home/team27/users_and_scores.db'
button_controls_db = '/var/jail/home/team27/button_controls.db'


def request_handler(request):
    if request['method'] == 'GET':
        user, score = request['values']['username'], request['values']['score']  # noqa: E501
        if 'new' in request['args']:
            updated_db = set_data(user, score, returning=False)
            if not updated_db:
                with open("/var/jail/home/team27/registered_user.html", "r") as f:
                    return f.read()
        elif 'returning' in request['args']:
            updated_db = set_data(user, score, returning=True)
            if not updated_db:
                with open("/var/jail/home/team27/user_not_found.html", "r") as f:
                    return f.read()
        elif 'leaderboard' in request['args']:
            with open("/var/jail/home/team27/leaderboard.html", "r") as f:
                return f.read()
        played_songs = get_songs(user)
        with open("/var/jail/home/team27/songlib.html", "r") as f:
            songlib = f.read()
            songlib = songlib.replace('<user.data>', user)
            if played_songs[0] == 1:
                songlib = songlib.replace('played_24?', "This song has been attempted!")
            else:
                songlib = songlib.replace('played_24?', "Try this new song!")
            if played_songs[1] == 1:
                songlib = songlib.replace('played_512?', "This song has been attempted!")
            else:
                songlib = songlib.replace('played_512?', "Try this new song!")
            if played_songs[2] == 1:
                songlib = songlib.replace('played_300?', "This song has been attempted!")
            else:
                songlib = songlib.replace('played_300?', "Try this new song!")
            if played_songs[3] == 1:
                songlib = songlib.replace('played_150?', "This song has been attempted!")
            else:
                songlib = songlib.replace('played_150?', "Try this new song!")
            return songlib

    else:
        return '<h1>Invalid request</h1>'


def create_database():
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS game_db
        (user text, score int, highscore int, played_24 int, played_512 int, played_300 int, played_150 int);""")  # noqa: E501

    conn.commit()
    conn.close()


def set_data(user, score, returning=False):
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute('''SELECT user, score, highscore FROM game_db WHERE user = ?''', (user,))   # noqa: E501
    result = c.fetchone()

    if returning:
        if result is None:
            conn.commit()
            conn.close()
            return False
        user_hs = get_highscore(user)[0]
        if int(score) > user_hs:
            c.execute('''UPDATE game_db SET score = (?), highscore = (?) WHERE user = (?)''', (score, score, user))   # noqa: E501
        else:
            c.execute("UPDATE game_db SET score = (?), highscore = (?) WHERE user = (?)", (score, user_hs, user))   # noqa: E501

    else:
        if result is not None:
            conn.commit()
            conn.close()
            return False
        c.execute("INSERT into game_db (user, score, highscore, played_24, played_512, played_300, played_150) VALUES (?, ?, ?, ?, ?, ?, ?)", (user, score, score, 0, 0, 0, 0))

    conn.commit()
    conn.close()
    return True


def get_highscore(user):
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    things = c.execute('''SELECT highscore FROM game_db WHERE user = (?)''', (user, )).fetchone()  # noqa: E501

    conn.commit()
    conn.close()

    return things


def get_top_scores():
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    things = c.execute('''SELECT user, highscore FROM game_db ORDER BY highscore ASC LIMIT 10''').fetchall()  # noqa: E501

    conn.commit()
    conn.close()

    return things


def get_songs(user):
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    things = c.execute('''SELECT played_24, played_512, played_300, played_150 FROM game_db WHERE user = (?)''', (user,)).fetchone()  # noqa: E501
    return things
