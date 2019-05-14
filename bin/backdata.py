# coding:utf-8
import os, sys
import config
import traceback
import time
import datetime
import logging

log = logging.getLogger()

backends = {}

class Backend (object):
    def __init__(self, path='', expire=300):
        self.path = path
        self.expire = expire

    def close(self):
        pass

    def get(self, key):
        pass

    def set(self, key):
        pass


import plyvel

class BackendLevelDB (Backend):
    def __init__(self, path='', expire=300):
        Backend.__init__(self, path, expire)
        self.db = plyvel.DB(self.path, create_if_missing=True)

    def close(self):
        self.db.close()

    def get(self, key):
        return self.db.get(key)

    def set(self, key, value):
        return self.db.put(key, value)

backends['leveldb'] = BackendLevelDB


try:
    import etcd

    class BackendEtcd (Backend):
        def __init__(self, addr, expire=300):
            Backend.__init__(self, path, expire)
            self.db = etcd.Client(host=addr[0], port=addr[1])

        def close(self):
            pass
            #self.db.close()

        def get(self, key):
            path = '/named/' + '/'.join(key.split('.'))
            ret = self.db.read(path)
            return ret.value

        def set(self, key, value):
            path = '/named/' + '/'.join(key.split('.'))
            return self.db.write(path, value)

    backends['etcd'] = BackendEtcd

except:
    log.info(traceback.format_exc())
    log.warn('not found etcd')

def create(name='leveldb'):
    global backends
    return backends[name](config.DATA_DIR)

