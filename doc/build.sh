#!/bin/bash

rm -rf ./html
rm -f ./*_dict.md

python3 extract_dict_properties.py

python_fragment=$(cat <<EOF
import os;
import re;
script_dir=os.path.dirname(os.path.realpath(__file__));
content='';
f=open(os.path.join(script_dir,'../src/rCFD_macros.h'),'r');
content=f.read();
major=int(re.search(r'#define\s+RCFD_VERSION_MAJOR\s+(\d+)',content).group(1));
minor=int(re.search(r'#define\s+RCFD_VERSION_MINOR\s+(\d+)',content).group(1));
print(f"{major}.{minor:02d}")
EOF
) ;

execute_python_fragment() {
  python3 <<< "$1"
}

version=$(execute_python_fragment "${python_fragment}")
echo result was $version

( cat Doxyfile ; echo "PROJECT_NUMBER=${version}" ) | doxygen -

python3 -m webbrowser ./html/index.html
