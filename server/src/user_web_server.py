import sqlite3

db = '/var/jail/home/team27/users_and_scores.db'
button_controls_db = '/var/jail/home/team27/button_controls.db'


def request_handler(request):
    if request['method'] == 'GET':
        # if 'sending_controls' in request['args']:
        #     op, cur_idx = get_controls()
        # cur_idx = 0

        with open("/var/jail/home/team27/login.html", "r") as f:
            return f.read()

    elif request['method'] == 'POST':
        if 'new' in request['form']:
            user, score = request['form']['username'], request['form']['score']
            updated_db = set_data(user, score, returning=False)
            if updated_db:
                return '<h1>{} Score Updated!:</h1> <p>Score: {}</p><br> \
                    <a href="./user_web_server.py">Change User/Game Score</a>'.format(user, score)
            else:
                return '<h1>This username is already registered. Please Login.</h1> <br> \
                    <a href="./user_web_server.py">Back to Login</a>'
        elif 'returning' in request['form']:
            user, score = request['form']['username'], request['form']['score']
            updated_db = set_data(user, score, returning=True)
            if updated_db:
                return '<h1>{} Score Updated!:</h1> <p>Score: {}</p><br> \
                    <a href="./user_web_server.py">Change User/Game Score</a>'.format(user, score)
            else:
                return '<h1>User not found. Please Register.</h1> <br> \
                    <a href="./user_web_server.py">Back to Login</a>'

        elif 'leaderboard' in request['form']:
            data = get_top_scores()
            return '<h1>Leaderboard:</h1> <p>Top 10: {}</p><br> \
                <a href="./user_web_server.py">Back to Login</a>'.format(data)
        return request['form']

    else:
        return '<h1>Invalid request</h1>'


def create_database():
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS game_db
        (user text, score int, highscore int);""")  # noqa: E501

    conn.commit()
    conn.close()


def set_data(user, score, returning=False):
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    c.execute('''SELECT user, score, highscore FROM game_db WHERE user = ?''', (user,))
    result = c.fetchone()

    if returning:
        if result is None:
            conn.commit()
            conn.close()
            return False
        user_hs = get_highscore(user)[0]
        if int(score) > user_hs:
            c.execute('''UPDATE game_db SET score = (?), highscore = (?) WHERE user = (?)''', (score, score, user))
        else:
            c.execute("UPDATE game_db SET score = (?), highscore = (?) WHERE user = (?)", (score, user_hs, user))

    else:
        if result is not None:
            conn.commit()
            conn.close()
            return False
        c.execute("INSERT into game_db VALUES (?, ?, ?)", (user, score, score))

    conn.commit()
    conn.close()
    return True


def get_highscore(user):
    # TODO: fix highscore implementation by adding timestamps and sorting in ascending order prior to getting

    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    things = c.execute('''SELECT highscore FROM game_db WHERE user = (?)''', (user, )).fetchone()

    conn.commit()
    conn.close()

    return things


def get_top_scores():
    create_database()
    conn = sqlite3.connect(db)
    c = conn.cursor()

    things = c.execute('''SELECT user, highscore FROM game_db ORDER BY highscore ASC LIMIT 10''').fetchall()

    conn.commit()
    conn.close()

    return things
