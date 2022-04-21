import sqlite3

db = '/var/jail/home/team27/beatsaber.db'


def request_handler(request):
    if request['method'] == 'GET':
        (dir, score) = get_data()
        return {'dir': str(dir),
                'score': int(score)}

    elif request['method'] == 'POST':
        dir = str(request['form']['dir'])
        dir_str = ""
        if dir[0] == '1':
            dir_str += "left, "
        if dir[1] == '1':
            dir_str += "right, "
        if dir[2] == '1':
            dir_str += "up, "
        if dir[3] == '1':
            dir_str += "down, "
        if len(dir_str) == 0:
            dir_str = "no move"
        score = get_score()
        set_data(dir, score)

        return {'dir': str(dir),
                'score': int(score)}

    else:
        return None


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


def get_score():
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute('''SELECT score FROM sensor_data''')
    score = c.fetchone()
    if score is None:
        return 0

    conn.close()

    return score[0]
