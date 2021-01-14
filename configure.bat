gdown --id 1dje9CA3gqaHD4Po5UrDZi3dhTeXuj3Ri -O libs.zip
gdown --id 1otyqcT-TohaHtv-rzguJn3lYCBBGXRVG -O models.zip

mkdir lib
mkdir models

tar -xf libs.zip -C lib
tar -xf models.zip -C models

del libs.zip
del models.zip