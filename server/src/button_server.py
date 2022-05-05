import sqlite3
button_controls_db = '/var/jail/home/team27/button_controls.db'


def request_handler(request):
    if request['method'] == 'GET':
        cur, old = get_controls()
        # return {"i": cur[1], "old": old[1], "op": cur[0]}
        # return "{\"i\": %s, \"old\": %s}" % (cur[1], old[1])
        return "{\"i\": %s, \"old\": %s, \"op\": \"%s\"}" % (cur[1], old[1], cur[0])


def create_database():
    conn = sqlite3.connect(button_controls_db)
    c = conn.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS controls_db
        (op text, cur_elem int, timing timestamp);""")  # noqa: E501

    conn.commit()
    conn.close()


def get_controls():
    create_database()
    conn = sqlite3.connect(button_controls_db)
    c = conn.cursor()

    things = c.execute('''SELECT op, cur_elem FROM controls_db ORDER BY timing DESC LIMIT 2''').fetchall()

    conn.commit()
    conn.close()

    return things
