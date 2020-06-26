from argparse import ArgumentParser
import shutil
from os import path as osp

argparser = ArgumentParser()
argparser.add_argument("src", type=str)
argparser.add_argument("dest", type=str)
argparser.add_argument("--overwrite", action="store_true", default=False)
arg = argparser.parse_args()

if arg.overwrite or not osp.exists(osp.join(arg.dest)):
    print(f"compressing {arg.src} into {arg.dest}", flush=True)
    shutil.make_archive(arg.dest, 'zip', arg.src)

