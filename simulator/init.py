import argparse
import json
import os
from subprocess import call


def parser():
    p = argparse.ArgumentParser()
    p.add_argument('effect')
    p.add_argument('path')
    p.add_argument('--hd', help="Render in HD", action='store_true')

    return p


def set_env(args):
    os.environ['effect'] = args.effect
    os.environ['path'] = args.path

    if args.hd:
        os.environ['hd'] = '1'


def write_fps(effect, path):
    with open(os.path.join(path, 'fps.json'), 'w') as f:
        p = os.path.join('exports', effect + '.json')
        d = json.load(open(p, 'r'))
        json.dump(
            {
                'fps': d['fps']
            },
            f
        )


def execute(args):
    set_env(args)
    os.chdir('..')
    call('scons --no-avr', shell=True)
    os.chdir('simulator')
    call('../build_exporter/exporter', shell=True)
    write_fps(args.effect, args.path)