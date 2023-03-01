from datetime import datetime, timedelta
import pytz
from flask import Flask, make_response


app = Flask(__name__)

@app.route('/')
def hello():
    gmt = pytz.timezone('GMT')
    response = make_response('Hello, world!')
    expires_at = datetime.now(gmt) + timedelta(seconds=20)
    #print('datetime.now(): '+datetime.now().strftime('%a, %d %b %Y %H:%M:%S GMT'))
    #print('expires_at: '+expires_at.strftime('%a, %d %b %Y %H:%M:%S GMT'))
    response.headers['Expires'] = expires_at.strftime('%a, %d %b %Y %H:%M:%S GMT')
    response.headers['Cache-Control'] = 'max-age=10, must-revalidate'
    response.headers['ETag'] = "abcd1234"
    response.headers['Last-Modified'] = "Thu, 24 Feb 2023 15:00:00 GMT"
    #response.headers['Expires'] = "Fri, 25 Feb 2023 11:30:00 GMT"
    return response

if __name__ == '__main__':
    app.run(debug=True, host = "0.0.0.0")