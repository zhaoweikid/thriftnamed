# vim: set ts=4 et sw=4 sts=4 fileencoding=utf-8 :
import os, sys

HOME = os.path.dirname(os.path.dirname(os.path.abspath("__file__")))
print(HOME)

# 服务地址
HOST = '0.0.0.0'

# 服务端口
PORT = 6200

# 调试模式: True/False
# 生产环境必须为False
DEBUG = False

# 日志文件配置
if DEBUG:
    LOGFILE = 'stdout'
else:
    LOGFILE = os.path.join(HOME, '../log/%s' % os.path.basename(HOME))


