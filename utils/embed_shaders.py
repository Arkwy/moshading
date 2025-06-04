# embed_shader.py
import sys
from pathlib import Path

output_cpp = sys.argv[1]
output_h = sys.argv[2]
input_folder = sys.argv[3]

with open(output_h, 'w') as h:
    h.write(f'#pragma once\n')
    with open(output_cpp, 'w') as cpp:
        cpp.write(f'#include "{output_h}"\n')

        for input_file in Path(input_folder).iterdir():
            if input_file.suffix != '.wgsl': continue

            var_name = '_'.join(input_file.name.split('.')[:-1])
            h.write(f'\nextern const char {var_name}[];\n')
            with open(input_file, 'r') as f:
                cpp.write(f'\nconst char {var_name}[] = R"__shader__({f.read()})__shader__";\n')
