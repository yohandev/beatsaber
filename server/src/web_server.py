import sqlite3

db = '/var/jail/home/team27/beatsaber.db'


def request_handler(request):
    if request['method'] == 'GET':
        (dir, score) = get_data()
        if 'js' in request['args']:
            return {'dir': str(dir),
                    'score': int(score)}
        return f'''
            <!DOCTYPE html>
            <html>
            <body>

            <h1>Game Info</h1>
            <form action="#" method="post">

            <h2>Current Status</h2>
            <p>Score: {score}</p>
            <p>Last Move: {dir}</p>

            <br>
            <label for="score">Score (between 0 and 20):</label>
            <input type="range" id="score" name="score" min="0" max="20" oninput="this.nextElementSibling.value = this.value">
            <output>0</output>
            <br>
            <input type="submit">
            </form>

            </body>
            </html>
            '''

    elif request['method'] == 'POST':
        dir = get_dir()
        score = int(request['form']['score'])
        set_data(dir, score)

        return '<h1>Game Score Updated!:</h1> <p>Score: {}</p><br> \
            <a href="./web_server.py">Change Game Score</a>'.format(score)

    else:
        return '<h1>Invalid request</h1>'


def create_database():
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS sensor_data
        (dir text, score int);""")  # noqa: E501

    conn.commit()
    conn.close()


def set_data(dir, score):
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute("DELETE FROM sensor_data")
    conn.commit()

    c.execute("INSERT into sensor_data VALUES (?, ?)", (dir, score))

    conn.commit()
    conn.close()


def get_data():
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute('''SELECT * FROM sensor_data''')
    data = c.fetchone()
    if data is None:
        return ("no move", 0)

    conn.close()

    return data


def get_dir():
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute('''SELECT dir FROM sensor_data''')
    dir = c.fetchone()
    if dir is None:
        return "no move"

    conn.close()

    return dir[0]
