import binascii
import struct
import time

import conf
import conv

def handle(resp):
    try:
        _responses[resp.kind](resp.body)
    except KeyError:
        print("Unknown response: {0}".format(resp))

def reboot(body):
    print('device rebooted')

def time_(timestamp):
    if len(timestamp) >= 4:
        dt = conv.longToInt(timestamp[:4])
        dtstr = time.strftime('%a %d.%m.%Y - %H:%M:%S', time.localtime(dt))
        t = int(time.time())
        print("device time off by {0} seconds ({1})".format(dt - t, dtstr))
    else:
        print("Received incorrect timestamp value from the device")

def effect_name(name):
    print 'Running effect "{0}"'.format(name)

def invalid_cmd(body):
    print('Received an invalid command, msg: {0}'.format(body))

def do_nothing(body):
    pass

_responses = {
    conf.RESP_REBOOT:      reboot,
    conf.RESP_TIME:        time_,
    conf.RESP_EFFECT_NAMES: effect_name,
    conf.RESP_INVALID_CMD: invalid_cmd,
    conf.RESP_EFFECT_END:  do_nothing,
    conf.RESP_COMMAND_OK:  do_nothing
}
