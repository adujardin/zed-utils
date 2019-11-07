#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
    echo "Usage : convert_svo.sh folder/of/svo/to/convert/ [compression_mode]"
fi

compression_mode = 4 #h264
if [ "$#" -eq 2 ]; then
    re='^[0-9]+$'
    if ! [[ $2 =~ $re ]] && [ $2 -gt 5 ]; then
        echo "Compression must be a number <= 5" >&2; exit 1
    fi
    compression_mode = "$2"
fi

converter_path="/usr/local/zed/tools/ZED_SVO_Editor" #"ZED_SVO_Editor"
input_folder="$1"
output_folder="$input_folder/converted/"
mkdir -p "$output_folder"

for svo_file in "$input_folder/"*".svo"; do
    echo $svo_file
    filename=$(basename -- "$svo_file")
    filename=$(echo "$filename" | cut -f 1 -d '.')
    "$converter_path" -cpr "$svo_file" -m $compression_mode "${output_folder}/${filename}_h264.svo"
done
