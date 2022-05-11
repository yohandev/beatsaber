import sqlite3

db = '/var/jail/home/team27/users_and_scores.db'
button_controls_db = '/var/jail/home/team27/button_controls.db'


def request_handler(request):
    if request['method'] == 'GET':
        with open("/var/jail/home/team27/login.html", "r") as f:
            return f.read()

    else:
        return '<h1>Invalid request</h1>'


def create_database():
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS game_db
        (user text, score int, highscore int, 24_played int, 512_played int, 300_played int, 150_played int);""")  # noqa: E501

    conn.commit()
    conn.close()


def get_highscore(user):
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    things = c.execute('''SELECT highscore FROM game_db WHERE user = (?)''', (user, )).fetchone()  # noqa: E501

    conn.commit()
    conn.close()

    return things
