#!/bin/bash
set -e

# Loop through all projects & build/publish (on m5burner)

mkdir -p dist

for i in src/*;do
	name=$(basename $i)
	pio run -e "${name}"
	cp .pio/build/${name}/firmware.bin dist/${name}.bin
	cp src/${name}/${name}.png dist/
	cp src/${name}/m5burner.json dist/${name}-m5burner.json
	npx -y m5-burner@latest publish-firmware dist/${name}-m5burner.json
done