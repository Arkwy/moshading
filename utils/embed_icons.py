import sys


def bin_to_c_array(icon_file, cpp_name, hpp_name):
    with open(icon_file, "rb") as icons:
        data = icons.read()

    with open(hpp_name, "w") as hpp:
        hpp.write("extern unsigned char icons_data[];\nextern unsigned int icons_data_len;")
        

    with open(cpp_name, "w") as cpp:
        cpp.write(f"unsigned char icons_data[] = {{")
        for i, b in enumerate(data):
            if i % 12 == 0:
                cpp.write("\n    ")
            cpp.write(f"0x{b:02x}, ")
        cpp.write("\n};\n")
        cpp.write(f"unsigned int icons_data_len = {len(data)};")



if len(sys.argv) < 4:
    print("Usage: python embed_icons.py <icon file> <.cpp name> <.hpp name>")
else:
    bin_to_c_array(sys.argv[1], sys.argv[2], sys.argv[3])


