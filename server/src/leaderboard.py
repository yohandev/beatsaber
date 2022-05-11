import sqlite3
import json

db = '/var/jail/home/team27/users_and_scores.db'


def request_handler(request):
    if request['method'] == 'GET':
        data = get_top_scores()

        lb = []
        for elm in data:
            lb.append(elm[0] + ": " + str(elm[1]))
        return json.dumps(lb[::-1])


def create_database():
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS game_db
        (user text, score int, highscore int);""")  # noqa: E501

    conn.commit()
    conn.close()


def get_top_scores():
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    things = c.execute('''SELECT user, highscore FROM game_db ORDER BY highscore ASC LIMIT 10''').fetchall()

    conn.commit()
    conn.close()

    return things
