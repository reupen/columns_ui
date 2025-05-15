cd /D "%~dp0"
docker compose run --build --rm builder npx -y svgo -f /mnt/assets/vectors -o /mnt/assets/vectors/minified
docker compose run --rm builder python3 convert-svg-to-ico.py --ignore dark-placeholder-artwork.svg light-placeholder-artwork.svg -- "vectors/*.svg" vectors/rendered
