# coding: utf-8
from gevent import monkey; monkey.patch_socket()
import os, sys
HOME = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.join(HOME, 'conf'))
import config
import logging
import traceback
import time
import datetime
import uuid
from zbase3.base import logger
from zbase3.server import rpc
import gevent
import backsync, backdata

log = logger.install(config.LOGFILE)

        
# key: token, value: name,ip
tokens = {}
db = backdata.create()

class NameHandler:
    def __init__(self, addr):
        self.addr = addr

    def ping(self, data=''):
        now = datetime.datetime.now()
        return 0, str(now)[:19]

    def query(self, name):
        retdata = {}
        if isinstance(name, list):
            for nm in name:
                retdata[nm] = self._get(nm)
        else:
            retdata[nm] = self._get(name)

        return OK, retdata

    def _get(self, key):
        # name, addr, weight, ctime
        global db

        ret = db.get(key)
        obj = json.loads(ret)
        
        now = int(time.time())

        retdata = []
        for row in obj:
            if now - row['ctime'] > config.EXPIRE:
                continue
            retdata.append(row)
        retdata.sort(key=lambda x:x['weight'], reverse=True)
        return retdata

    
    def _set(self, key, value):
        # name, addr, weight, ctime
        global db

        ret = db.get(key)
        obj = json.loads(ret)

        now = value['ctime']
        newobj = []
        for row in obj:
            if row['addr'] == value['addr']:
                row['ctime'] = now
                row['weight'] = value['weight']
            if now - row['ctime'] > 3600:
                continue
            newobj.append(row)

        newobj.append(row)
        
        data = json.dumps(newobj)
        db.put(key, data)
 

    def report(self, name, addr, weight):
        now = int(time.time())

        row = {'addr':addr, 'weight':weight, 'ctime':now}
        self._set(name, row)
        return OK, {name:row}

    # for server
    def auth(self, key):
        ac = backsync.AESCrypt(config.AUTH_KEY)
        s = ac.dec(key)
        group_name, server_name = s.split('|')
        if group_name != config.GROUP_NAME:
            return ERR_AUTH, 'group name error'
        
        names = [ x['name'] for x in config.GROUP_SERVER ]
        if server_name not in names:
            return ERR_AUTH, 'server name error'

        global tokens
        deltoken = None
        for k,v in tokens.items():
            if v[0] == server_name: # exist
                deltoken = k
                break
        if deltoken:
            tokens.pop(deltoken)

        token = uuid.uuid4().hex

        tokens[token] = (server_name, '')
        return OK, token


    # for server
    def sync(self, token, name, addr, weight, ctime):
        global tokens
        now = int(time.time())
        
        tinfo = tokens.get(token)
        if not tinfo or tinfo[0] != name:
            return ERR_AUTH, 'token error'

        if now - ctime > 60:
            return ERR, 'too old, %d' % (now-ctime)

        row = {'addr':addr, 'weight':weight, 'ctime':now}
        self._set(name, row)
        return OK, {name:row}


if __name__ == "__main__":
    if config.GROUP_SYNC:
        gevent.spawn(backsync.sync_servers)
    rpc.gevent_server(config.PORT, NameHandler, proto='tcp,udp')


