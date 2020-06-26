from argparse import ArgumentParser
from io import BytesIO
from urllib.request import urlopen
from zipfile import ZipFile
import os.path as osp


argparser = ArgumentParser()
argparser.add_argument("--bds-version", type=str, default="1.14.60.5")
argparser.add_argument("--to", type=str, default="./")
argparser.add_argument("--overwrite", action="store_true", default=False)
arg = argparser.parse_args()

if arg.overwrite or not osp.exists(osp.join(arg.to, "bedrock_server.exe")):
    bds_url = f'https://minecraft.azureedge.net/bin-win/bedrock-server-{arg.bds_version}.zip'
    print(f"downloading bedrock server from: {bds_url}", flush=True)
    with urlopen(bds_url) as zipresp:
        with ZipFile(BytesIO(zipresp.read())) as zfile:
            zfile.extractall(arg.to)
