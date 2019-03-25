# coding: utf-8
import os, sys
HOME = os.path.join(os.path.dirname(os.path.abspath(__file__)))
import config
import tornado
import logging
import struct
import traceback
from tornado.ioloop import IOLoop
from tornado import gen
from tornado.iostream import StreamClosedError
from tornado.tcpserver import TCPServer
from tornado.options import options, define
from qfcommon3.base import logger

log = logger.install(config.LOGFILE)

define("port", default=9888, help="TCP port to listen on")
logger = logging.getLogger(__name__)


class ThriftNameServer(TCPServer):
    @gen.coroutine
    def handle_stream(self, stream, address):
        while True:
            try:
                data = yield stream.read_bytes(4)
                log.info("Received bytes: %s", data)
                bodylen = struct.unpack('I', data)
                if bodylen > 0:
                    data = yield stream.read_bytes(bodylen)
                    #yield stream.write(data)
            except StreamClosedError:
                logger.warning("Lost client at host %s", address[0])
                break
            except Exception as e:
                log.warn(traceback.format_exc())


if __name__ == "__main__":
    options.parse_command_line()
    server = ThriftNameServer()
    server.listen(options.port)
    logger.info("Listening on TCP port %d", options.port)
    IOLoop.current().start()



