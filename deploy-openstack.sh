#!/usr/bin/env bash -e

module load python/3.5.1
pyvenv .venv
source .venv/bin/activate
pip install python-openstackclient

mkdir artifacts

cd Scripts
./CreateBuilderImage
./BringUpQueue