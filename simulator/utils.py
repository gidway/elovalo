import argparse
import json
import os
from subprocess import call


def effect_parser():
    p = parser()
    p.add_argument('effect', help='name of the effect to render')
    p.add_argument('output', help='output path')

    return p


def parser():
    p = argparse.ArgumentParser()
    p.add_argument('--hd', help='render in HD', action='store_true')

    return p


def export(effect, output, length='100'):
    length = os.environ.get('length', length) or length
    length = str(length)

    set_env(effect, output)

    os.chdir('..')
    call('scons --no-avr', shell=True)
    os.chdir('simulator')
    # TODO: use effect name here too!
    call(['../build/exporter/exporter ' + length], shell=True)
    write_fps(effect, output)


def set_env(effect, output):
    os.environ['effect'] = effect
    os.environ['output'] = output


def write_fps(effect, output):
    if not os.path.exists(output):
        os.mkdir(output)

    with open(os.path.join(output, 'fps.json'), 'w') as f:
        p = os.path.join('exports', effect + '.json')
        d = json.load(open(p, 'r'))
        json.dump(
            {
                'fps': d['fps']
            },
            f
        )
