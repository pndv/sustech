import flask, requests, json
import jinja2
from authlib.flask.client import OAuth
from flask import jsonify
from flask_sqlalchemy import SQLAlchemy

app = flask.Flask(__name__)

app.config["SQLALCHEMY_DATABASE_URI"] = 'sqlite:///test.db'
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = 'False'
db = SQLAlchemy(app)
db.create_all()


class Info(db.model):
    id = db.Column(db.Integer, primary_key=True)
    temp = db.Column(db.String(5), unique=False, nullable=False)
    status = db.Column(db.String(3), unique=False, nullable=False)


@app.route("/")
def home_view():
    to_return = ""
    allInfo = Info.query.all()
    for infoBit in allInfo:
        to_return += "<p> Status: %s, Temperature: %s</p><br/>" % (infoBit.status, infoBit.temp)
    return to_return


@app.route("/latest.html", methods=["GET"])
def latest():
    latest_info = Info.query.order_by(Info.id.desc()).first()
    if latest_info is None:
        return flask.jsonify(info="No Data", status=200)
    else:
        return flask.jsonify(info=latest_info.status + ";" + latest_info.temp, status=200)


@app.route("/toggle")
def toggle_manual():
    old_info = Info.query.order_by(Info.id.desc()).first()
    if old_info is None:
        temperature = "0"
        stat = "0"
    else:
        temperature = old_info.temp
        stat = old_info.status

    if stat == "0":
        stat = "1"
    else:
        stat = "0"

    new_info = Info(temp=temperature, status=stat)
    db.session.add(new_info)
    db.session.commit()

    return flask.jsonify(status=200)


@app.route("/toggle.html", methods=["POST"])
def toggle():
    old_info = Info.query.order_by(Info.id.desc()).first()
    if old_info is None:
        temperature = "0"
        stat = "0"
    else:
        temperature = old_info.temp
        stat = old_info.status

    if stat == "0":
        stat = "1"
    else:
        stat = "0"

    new_info = Info(temp=temperature, status=stat)
    db.session.add(new_info)
    db.session.commit()

    return flask.jsonify(status=200)


@app.route("/temp.html", methods=["POST"])
def temp():
    new_temp = list(flask.request.form.keys())[0]
    old_info = Info.query.order_by(Info.id.desc()).first()

    if old_info is None:
        stat = "0"
    else:
        stat = old_info.status

    new_info = Info(temp=new_temp, status=stat)
    db.session.add(new_info)
    db.session.commit()

    return flask.jsonify(status=200)
