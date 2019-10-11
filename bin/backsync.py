# coding:utf-8
import os, sys
from Crypto.Cipher import AES
import base64
from zbase3.server.rpc import RPCClient
from zbase3.server.defines import *
import queue
import traceback
import time
import datetime
import gevent
import logging
import config

log = logging.getLogger()

servers = {}

q = queue.Queue()
 
class AESCrypt():
    def __init__(self, key):
        self.key = key
        self.mode = AES.MODE_CBC
     
    def enc(self, text):
        cryptor = AES.new(self.key, self.mode, self.key[:16])
        if isinstance(text, str):
            text = text.encode('utf-8')
        text = text + (b'\0' * (16-len(text)%16))
        ctext = cryptor.encrypt(text)
        return base64.b64encode(ctext)
     
    def dec(self, text):
        cryptor = AES.new(self.key, self.mode, self.key[:16])
        plain_text = cryptor.decrypt(base64.b64decode(text))
        return plain_text.rstrip(b'\0').decode('utf-8')

'''
class SyncServer:
    def __init__(self):
        # 已经建立连接的server信息
        self.servers = {}
        # 需要连接的server数量
        self.server_count = 0
        self.ac = AESCrypt(config.AUTH_KEY)

        self.connect_all()

    def connect_all(self):
        for x in config.GROUP_SERVER:
            if x['name'] == config.SERVER_NAME: # skip
                continue
            self.server_count += 1
            try:
                self.connect(x['name'], x['addr'])
            except:
                log.warn('connect %s %s error' % (x['name'], x['addr']))
                log.warn(traceback.format_exc())


    def connect(self, name, addr):
        authstr = self.ac.enc('%s|%s' % (config.GROUP_NAME, config.SERVER_NAME)).decode('utf-8')
        
        c = RPCClient({'addr':addr, 'timeout':1000})
        ret, token = c.auth(key=authstr)
    
        self.servers[name] = {'addr':addr, 'client':c, 'token':token}
        return c


    def check_server(self):
        delnames = []
        for x in config.GROUP_SERVER:
            name = x['name']
            # 忽略自己
            if name == config.SERVER_NAME:
                continue
            # 已连接上的，调用ping
            if name in self.servers:
                try:
                    self.servers[name]['client'].ping(data='ping')
                except:
                    delnames.append(name)
                    log.warn(traceback.format_exc())
                continue
            # 还未连接的，连接上之后ping
            try:
                c = self.connect(name, x['addr'])
                c.ping(data='ping')
            except Exception as e:
                log.warn('connect %s %s error: %s' % (name, x['addr'], str(e)))
                log.warn(traceback.format_exc())

        # 出了异常的关闭掉，下次重连
        for nm in delnames:
            log.info('close %s', nm)
            self.servers[nm]['client']._close()
            self.servers.pop(nm)

    def sendall(self, msg):
        dels = []
        
        if len(self.servers) != self.server_count:
            self.check_server()

        for name,serv in self.servers.items():
            try:
                log.debug('send %s to %s', msg, serv['addr'])
                msg['token'] = serv['token']
                serv['client'].sync(**msg)
            except:
                dels.append(name)
                log.warn(traceback.format_exc())

        for nm in dels:
            log.info('close %s', nm)
            self.servers[nm]['client']._close()
            self.servers.pop(nm)
                
    def run(self):
        global q
        while True:
            log.info('sync queue len:%d', q.qsize())
            now = int(time.time())
            try:
                try:
                    msg = q.get(True, 10)
                except queue.Empty: 
                    self.check_server()
                    continue

                if now - msg['ctime'] > 30:
                    log.debug('msg too old, skip %s', msg)
                    continue

                self.sendall(msg)
            except KeyboardInterrupt:
                raise
            except:
                log.warn(traceback.format_exc())

def sync_servers_old():
    log.info('group sync on')
    syncer = SyncServer()
    syncer.run()
'''


class SyncOneServer:
    def __init__(self, conf):
        # 已经建立连接的server信息
        self.name = conf['name']
        self.addr = conf['addr']
        self.ac = AESCrypt(config.AUTH_KEY)
        self.authstr = self.ac.enc('%s|%s' % (config.GROUP_NAME, config.SERVER_NAME)).decode('utf-8')
        self.token = ''
        self.c = None
        self.q = queue.Queue()

        if not self.addr:
            raise ValueError('server name %s error' % name)

    def push(self, msg):
        msg['ctime'] = int(time.time())
        log.debug('push %s %s', self.name, msg)
        self.q.put(msg)

    def connect(self):
        self.c = RPCClient({'addr':self.addr, 'timeout':1000})
        ret, token = self.c.auth(key=self.authstr)
        if ret == OK:
            self.token = token

    def check_server(self):
        try:
            if not self.c:
                self.connect()
            self.c.ping(data='ping')
        except:
            self.c._close()
            self.c = None
            log.warn(traceback.format_exc())

    def send(self, msg):
        msg['token'] = self.token
        try:
            log.debug('msg:%s', msg)
            self.c.sync(**msg)
        except:
            self.c._close()
            self.c = None
            log.warn(traceback.format_exc())
                
    def run(self):
        log.debug('start run sync %s', self.name)
        while True:
            try:
                if not self.c:
                    self.connect()

                log.info('%s sync len:%d', self.name, self.q.qsize())
                now = int(time.time())

                try:
                    msg = self.q.get(True, 10)
                except queue.Empty: 
                    self.check_server()
                    continue
                # 超过30s的数据已经太旧了，丢弃
                log.debug('get msg:%s', msg)
                if now - msg['ctime'] > config.EXPIRE:
                    log.debug('msg too old, skip %s', msg)
                    continue
                self.send(msg)
            except KeyboardInterrupt:
                raise
            except:
                gevent.sleep(1)
                log.warn(traceback.format_exc())


def push(msg):
    global servers

    for name,serv in servers.items():
        serv.push(msg)

def run_sync(server):
    server.run()

def sync_servers():
    if not config.GROUP_SYNC:
        return
   
    global servers
    for one in config.GROUP_SERVER:
        if one['name'] == config.SERVER_NAME:
            continue
        s = SyncOneServer(one)
        servers[one['name']] = s
        gevent.spawn(run_sync, s)
    



