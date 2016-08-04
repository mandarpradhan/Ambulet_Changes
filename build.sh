#Check for root access
if [ "$(id -u)" != "0" ]; then
  echo "SeaMo requires root user permissions"
  exit 1
fi

cd src/seamo_prehandoff
make

cd -

cd src/seamo_vho_core
make

cd -

cd src/seamo_vrms
make

cd -
