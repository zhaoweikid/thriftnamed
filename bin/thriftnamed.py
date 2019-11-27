# coding: utf-8
from gevent import monkey; monkey.patch_all()
import os, sys
HOME = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.join(HOME, 'conf'))

from zbase3.base import logger,loader
if __name__ == '__main__':
    loader.loadconf_argv(HOME)
else:
    loader.loadconf(HOME)

import config
import logging
import traceback
import time
import datetime
import uuid
import json
from zbase3.server import rpc
from zbase3.server.defines import *
import gevent
import backsync, backdata

log = logger.install(config.LOGFILE)

ERR_NOT_READY = -1000
errmsg[ERR_NOT_READY] = 'server not ready, please try later'
        
# key: token, value: name,ip
tokens = {}
backdata.create()


def check_token(func):
    def _(self, *args, **kwargs):
        global tokens
        token = kwargs.get('token')
        
        tinfo = tokens.get(token)
        log.debug('token %s %s', token, tinfo)
        if not tinfo:
            return ERR_AUTH, 'token error 1'
        if tinfo['addr'][0] != self.addr[0]:
            return ERR_AUTH, 'token error 2'

        return func(self, *args, **kwargs)
    return _


class DBAction:
    def _get(self, key):
        # name, addr, weight, ctime
        if isinstance(key, str):
            key = key.encode('utf-8')

        ret = backdata.db.get(key)
        log.debug('get %s %s', key, ret)
        if not ret:
            return None
        obj = json.loads(ret)
        
        now = int(time.time())

        retdata = []
        for row in obj:
            if now - row['ctime'] > config.EXPIRE:
                continue
            retdata.append(row)
        retdata.sort(key=lambda x:x['weight'], reverse=True)
        log.debug('db get %s %s', key, retdata)
        return retdata

    
    def _set(self, key, value):
        # name, addr, weight, ctime
        log.debug('set key:%s value:%s', key, value)

        if isinstance(key, str):
            key = key.encode('utf-8')
        
        obj = []
        ret = backdata.db.get(key)
        if ret:
            obj = json.loads(ret)
        log.debug('db:%s', obj)
        now = value['ctime']
        newobj = []
        addr_exist = False
        for row in obj:
            if row['server']['addr'] == value['server']['addr']:
                # 如果数据库里的时间比传入的要新，不需要更新数据
                if row['rtime'] >= value['rtime']:
                    log.info('record time db:%d in:%d', row['rtime'], value['rtime'])
                    return

                row['ctime'] = now
                row['weight'] = value['weight']
                addr_exist = True

            if now - row['ctime'] > config.EXPIRE:
                continue
            newobj.append(row)
    
        if not addr_exist:
            newobj.append(value)
        
        data = json.dumps(newobj)
        log.debug('db set %s %s', key, data.encode('utf-8'))
        backdata.db.set(key, data.encode('utf-8'))

    def _set_data(self, data):
        for key,value in data:
            backdata.db.set(key, value)
 
    def _delete(self, name, addr):
        # name, addr

        if isinstance(name, str):
            name = name.encode('utf-8')

        if not addr:
            backdata.db.remove(name)
            return OK, {'name':name}
    
        ret = backdata.db.get(name)
        if not ret:
            return OK, {'name':name}

        obj = json.loads(ret)
        newobj = []
        for row in obj:
            if now - row['ctime'] > config.EXPIRE:
                continue
            server = row['server']
            if server['addr'] != addr:
                newobj.append(row)
   
        if not newobj:
            backdata.db.remove(name)
            return OK, {'name':name}
        
        if len(newobj) != len(obj):
            data = json.dumps(newobj)
            log.debug('db set %s %s', key, data.encode('utf-8'))
            backdata.db.set(name, data.encode('utf-8'))

        return OK, {'name':name}
        


class NameHandler (DBAction):
    def __init__(self, addr):
        self.addr = addr

    def ping(self, data=''):
        now = datetime.datetime.now()
        return 0, {'name':config.SERVER_NAME, 'group':config.GROUP_NAME, 'time':str(now)[:19]}

    def query(self, name):
        retdata = {}
        if isinstance(name, list):
            for nm in name:
                retdata[nm] = self._get(nm)
        else:
            retdata[name] = self._get(name)
        return OK, retdata

    def report(self, name, server, weight, rtime):
        now = int(time.time())

        row = {'server':server, 'weight':weight, 'ctime':now, 'rtime':rtime}
        self._set(name, row)
        row['name'] = name
        backsync.push({'method':'report', 'data':row})
        return OK, {name:row}

    def remove(self, name, addr):
        # name, addr
        self._delete(name, addr)
        backsync.push({'method':'remove', 'data':{'name':name, 'addr':addr}})

    # for server
    def auth(self, key):
        ac = backsync.AESCrypt(config.AUTH_KEY)
        s = ac.dec(key)
        group_name, server_name = s.split('|')
        if group_name != config.GROUP_NAME:
            return ERR_AUTH, 'group name error'
        
        if server_name not in config.GROUP_SERVER_MAP:
            return ERR_AUTH, 'server name error'

        global tokens
        deltoken = None
        for k,v in tokens.items():
            if v['name'] == server_name: # exist
                deltoken = k
                break
        if deltoken:
            tokens.pop(deltoken)

        token = uuid.uuid4().hex

        tokens[token] = {'name':server_name, 'addr':self.addr, 'time':int(time.time())}
        return OK, token

    # for server
    @check_token
    def sync(self, token, method, data, ctime):
        now = int(time.time())
        if now - ctime > 60:
            return ERR, 'too old, %d' % (now-ctime)

        if method == 'report':
            name = data['name']
            data.pop('name')
            self._set(name, data)
            return OK, {}
        elif method == 'remove':
            self._delete(**data)
            return OK, {}

    # for server
    @check_token
    def getall(self):
        data = backdata.db.getall()
        return OK, data

def load_data():
    dba = DBAction()
    for one in config.GROUP_SERVER:
        if one['name'] == config.SERVER_NAME:
            continue
        log.info('load data from:%s', one)
        s = backsync.SyncOneServer(one)
        try:
            s.connect()            
            ret,data = s.c.getall()
            if data:
                dba._set_data(data)
            log.debug('load data:%s', len(data))
        except Exception as e:
            log.info('load data error:%s', e)
        finally:
            s.close()



def main():
    if len(sys.argv) > 2:
        config.PORT = int(sys.argv[2])
    log.info('backdata nodata:%s', backdata.db.nodata)
    if backdata.db.nodata:
        load_data()

    if config.GROUP_SYNC:
        backsync.sync_servers()
    rpc.gevent_server(config.PORT, NameHandler, proto='tcp,udp')


if __name__ == "__main__":
    main()

