import sqlite3
import requests
import datetime

button_controls_db = '/var/jail/home/team27/button_controls.db'
web_server_url = 'https://608dev-2.net/sandbox/sc/team27/user_web_server.py'


def request_handler(request):
    if request['method'] == 'GET':
        return {'in_game_state': 0}

    elif request['method'] == 'POST':
        op = str(request['form']['op'])
        set_data(op)  # add to database

        return f"{op} added to database"

    else:
        return None


def create_database():
    conn = sqlite3.connect(button_controls_db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS controls_db
        (op text, cur_elem int, timing timestamp);""")  # noqa: E501

    conn.commit()
    conn.close()


def set_data(op):
    create_database()
    conn = sqlite3.connect(button_controls_db)
    c = conn.cursor()

    cur_elem = get_cur_elem()

    if op == 'right':
        cur_elem += 1
    elif op == 'left':
        cur_elem -= 1

    elif op == 'reset':
        cur_elem = 0

    c.execute("INSERT into controls_db VALUES (?, ?, ?)", (op, cur_elem, datetime.datetime.now()))

    conn.commit()
    conn.close()


def get_cur_elem():
    create_database()
    conn = sqlite3.connect(button_controls_db)
    c = conn.cursor()

    things = c.execute('''SELECT cur_elem FROM controls_db ORDER BY timing DESC LIMIT 1;''').fetchone()

    conn.commit()
    conn.close()

    if things is None:
        return 0
    else:
        return things[0]
