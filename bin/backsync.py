# coding:utf-8
import os, sys
from Crypto.Cipher import AES
import base64
from zbase3.server.rpc import RPCClient
import queue
import traceback
import time
import datetime
import logging
import config

log = logging.getLogger()

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


class SyncServer:
    def __init__(self):
        self.servers = {}
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
        token = c.auth(authstr)
    
        self.servers[name] = {'addr':addr, 'client':c, 'token':token}
        return c


    def check_server(self):
        if len(self.servers) == self.server_count:
            return

        delnames = []
        for x in config.GROUP_SERVER:
            name = x['name']
            if name == config.SERVER_NAME:
                continue
            if name in self.servers:
                try:
                    self.servers[name]['client'].ping('ping')
                except:
                    delnames.append(name)
                    log.warn(traceback.format_exc())
                continue
            try:
                c = self.connect(name, x['addr'])
                c.ping('ping')
            except:
                log.warn('connect %s %s error' % (name, x['addr']))
                log.warn(traceback.format_exc())

        for nm in delnames:
            self.servers[nm]['client'].close()
            self.servers.pop(nm)

    def pingall(self):
        dels = []
        for name,serv in self.servers.items():
            try:
                log.debug('ping %s', serv['addr'])
                serv['client'].ping('ping')
            except:
                log.warn(traceback.format_exc())
                dels.append(name)

        for nm in dels:
            self.servers[nm]['client'].close()
            self.servers.pop(nm)

    def sendall(self, msg):
        for name,serv in self.servers.items():
            try:
                log.debug('send %s to %s', msg, serv['addr'])
                msg['token'] = serv['token']
                serv['client'].sync(**msg)
            except:
                log.warn(traceback.format_exc())
                
    def run(self):
        global q
        while True:
            log.info('queue len:%d', q.qsize())
            now = int(time.time())
            try:
                try:
                    msg = q.get(True, 3)
                except queue.Empty: 
                    self.check_server()
                    continue

                if now - msg['ctime'] > 30:
                    log.debug('msg too old, skip %s', msg)
                    continue

                while True:
                    self.check_server()
                    if not self.servers:
                        log.warn('no server in group %s !!!', config.GROUP_NAME)
                        gevent.sleep(2)
                        continue
                    break

                self.sendall(msg)
            except KeyboardInterrupt:
                raise
            except:
                log.warn(traceback.format_exc())


def sync_servers():
    log.info('group sync on')
    syncer = SyncServer()
    syncer.run()


