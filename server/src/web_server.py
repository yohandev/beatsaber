import sqlite3
import datetime

db = '/var/jail/home/team27/beatsaber.db'
now = datetime.datetime.now()


def request_handler(request):
    if request['method'] == 'GET':
        with sqlite3.connect(db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS sensor_data (time_ timestamp, direction text, score int);""")  # noqa: E501
            all_data = c.execute("SELECT * FROM sensor_data")

            data = all_data.fetchone()
        # return data
        (time, dir, score) = data
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
        score_num = int(request['form']['score'])
        with sqlite3.connect(db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS sensor_data (time_ timestamp, direction text, score int DEFAULT 0 NOT NULL);""")
            data = c.execute('''SELECT time_, direction FROM sensor_data''')
            (time, direction) = data.fetchone()
            c.execute("INSERT INTO sensor_data VALUES (?, ?, ?)", (time, direction, score_num))

        return '<h1>Game Score Updated!:</h1> <p>Score: {}</p><br> \
            <a href="./web_server.py">Change Game Score</a>'.format(score_num)

    else:
        return '<h1>Invalid request</h1>'
