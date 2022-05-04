import sqlite3

db = '/var/jail/home/team27/users_and_scores.db'


def request_handler(request):
    if request['method'] == 'GET':
        return '''
            <!DOCTYPE html>
            <html>
            <body>

            <h1>Welcome to Beatsaber!</h1>
            <form action="#" method="post">

            <h2>Returning or Registering User: </h2>

            <br>
            Username:
            <input type="text" id="user" name="username">
            <br>

            <br>
            <label for="score">Set User Score(between 0 and 20):</label>
            <input type="range" id="score" name="score" min="0" max="20" oninput="this.nextElementSibling.value = this.value">
            <output>10</output>
            <br>

            <button type="submit" name="returning" value="True">
                Login
            </button>

            <button type="submit" name="new" value="True" formaction="?new_user">
                Register User
            </button>

            <h2>See Leaderboard: </h2>
            <button type="submit" name="leaderboard" value="True" formaction="?leaderboard">
                Leaderboard
            </button>
            </form>

            </body>
            </html>
            '''

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
