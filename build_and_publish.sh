#!/bin/bash

# exit on error
# set -e

# Loop through all projects & build/publish (on m5burner)
# run with "publish" arg to publish

mode="${1}"

mkdir -p dist

for i in src/*;do
	name=$(basename $i)
	title="$(jq -r .fields.name src/${name}/m5burner.json)"
	echo "Building ${title}"
	pio run -e "${name}"
	echo "Copying ${title}"
	cp .pio/build/${name}/firmware.bin dist/${name}.bin
	cp src/${name}/${name}.png dist/
	cp src/${name}/m5burner.json dist/${name}-m5burner.json
	if [ "${mode}"  == "publish" ];then
		echo "Publishing ${title}"
		npx -y m5-burner@latest publish-firmware dist/${name}-m5burner.json
	fi
done