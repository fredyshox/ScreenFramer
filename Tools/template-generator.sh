#!/bin/sh

function signal() {
  exit 2
}
trap signal SIGINT

TEMPLATE_DIR=$1
RESOURCE_DIR=$2
CONTENT_JSON="$2/contents.json"
if [ $# -lt 2 ] || [ ! -d $TEMPLATE_DIR ] || [ ! -d $RESOURCE_DIR ]; then
  echo "Usage: $0 template_directory resource_directory"
  exit 1
fi

# clear json file
echo '{}' >"${CONTENT_JSON}" 

# apple psd
echo "*** APPLE PSD"
for PSD in "${TEMPLATE_DIR}"/**/*.psd; do 
  if [ -f "${PSD}" ]; then
    PNG="${RESOURCE_DIR}/$(basename "${PSD}" .psd).png"
    echo "*** Loading PSD at: ${PSD}"
    python3 ./Tools/apple-psd2png.py --db "${CONTENT_JSON}" "${PSD}" "${PNG}"
    echo "*** Done. Output PNG at: ${PNG}"
  fi
done
# fastlane frameit
echo ""
echo "*** FASTLANE FRAMEIT"
python3 ./Tools/fastlane-templates.py "${RESOURCE_DIR}" "${CONTENT_JSON}"


echo "\n*** Template database at: ${CONTENT_JSON}"

