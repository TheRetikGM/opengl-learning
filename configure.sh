#!/bin/bash

gdown --id 1dje9CA3gqaHD4Po5UrDZi3dhTeXuj3Ri -O libs.zip
gdown --id 1otyqcT-TohaHtv-rzguJn3lYCBBGXRVG -O models.zip

mkdir lib models

unzip libs.zip -d lib
unzip models.zip -d models

rm libs.zip
rm models.zip