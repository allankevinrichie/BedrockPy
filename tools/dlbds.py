from argparse import ArgumentParser
from io import BytesIO
from urllib.request import urlopen
from zipfile import ZipFile


argparser = ArgumentParser()
argparser.add_argument("--bds-version", type=str, default="1.14.60.5")
argparser.add_argument("--to", type=str, default="./")
arg = argparser.parse_args()

bds_url = f'https://minecraft.azureedge.net/bin-win/bedrock-server-{arg.bds_version}.zip'
with urlopen(bds_url) as zipresp:
    with ZipFile(BytesIO(zipresp.read())) as zfile:
        zfile.extractall(arg.to)
