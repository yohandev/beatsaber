import sqlite3
import datetime

db = '/var/jail/home/team27/beatsaber.db'
now = datetime.datetime.now()


def request_handler(request):
    if request["method"] == "POST":
        dir = request['form']['dir']

        with sqlite3.connect(db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS sensor_data (time_ timestamp, direction text, score int);""")  # noqa: E501
            c.execute("INSERT INTO sensor_data (time_, direction) VALUES (?, ?)", (datetime.datetime.now(), dir))  # noqa: E501

        return "Data POSTED successfully"

    elif request['method'] == 'GET':
        with sqlite3.connect(db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS sensor_data (time_ timestamp, direction text, score int);""")  # noqa: E501
            c.execute("SELECT * FROM sensor_data")
            data = c.fetchone()
        return {
            'time': data[0],
            'dir': data[1],
            'score': data[2]
        }
    else:
        return "Invalid HTTP method for this url."
