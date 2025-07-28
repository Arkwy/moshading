# import sys


# def bin_to_c_array(icon_file, cpp_name, hpp_name):
#     with open(icon_file, "rb") as icons:
#         data = icons.read()

#     with open(hpp_name, "w") as hpp:
#         hpp.write("extern unsigned char icons_data[];\nextern unsigned int icons_data_len;")
        

#     with open(cpp_name, "w") as cpp:
#         cpp.write(f"unsigned char icons_data[] = {{")
#         for i, b in enumerate(data):
#             if i % 12 == 0:
#                 cpp.write("\n    ")
#             cpp.write(f"0x{b:02x}, ")
#         cpp.write("\n};\n")
#         cpp.write(f"unsigned int icons_data_len = {len(data)};")



# if len(sys.argv) < 4:
#     print("Usage: python embed_icons.py <icon file> <.cpp name> <.hpp name>")
# else:
#     bin_to_c_array(sys.argv[1], sys.argv[2], sys.argv[3])


import sys
import subprocess
import tempfile
import os


def bin_to_c_array(binary_data, cpp_name, hpp_name):
    with open(hpp_name, "w") as hpp:
        hpp.write("extern unsigned char icons_data[];\nextern unsigned int icons_data_len;\n")

    with open(cpp_name, "w") as cpp:
        cpp.write("unsigned char icons_data[] = {")
        for i, b in enumerate(binary_data):
            if i % 12 == 0:
                cpp.write("\n    ")
            cpp.write(f"0x{b:02x}, ")
        cpp.write("\n};\n")
        cpp.write(f"unsigned int icons_data_len = {len(binary_data)};\n")


def subset_font(original_path, subset_text):
    with tempfile.NamedTemporaryFile(delete=False, suffix=".ttf") as tmp_font:
        tmp_font_path = tmp_font.name

    # Run pyftsubset to generate a font with only specified glyphs
    try:
        subprocess.run([
            "pyftsubset",
            original_path,
            f"--text={subset_text}",
            f"--output-file={tmp_font_path}",
            "--layout-features=''",  # Disable OpenType features (smaller)
        ], check=True)

        with open(tmp_font_path, "rb") as f:
            subset_data = f.read()

        os.remove(tmp_font_path)
        return subset_data

    except subprocess.CalledProcessError as e:
        print("Error: pyftsubset failed:", e)
        sys.exit(1)


def main():
    if len(sys.argv) < 4:
        print("Usage: python embed_icons.py <icon file> <.cpp name> <.hpp name> [--subset \"chars\"]")
        return

    icon_file = sys.argv[1]
    cpp_name = sys.argv[2]
    hpp_name = sys.argv[3]

    subset_text = None
    if "--subset" in sys.argv:
        idx = sys.argv.index("--subset")
        if idx + 1 < len(sys.argv):
            subset_text = sys.argv[idx + 1]
        else:
            print("Error: --subset provided without characters.")
            return

    if subset_text:
        print(f"Subsetting font to only include: {repr(subset_text)}")
        binary_data = subset_font(icon_file, subset_text)
    else:
        print("Embedding full icon file.")
        with open(icon_file, "rb") as f:
            binary_data = f.read()

    bin_to_c_array(binary_data, cpp_name, hpp_name)
    print(f"Done: embedded {len(binary_data)} bytes into {cpp_name} and {hpp_name}")


if __name__ == "__main__":
    main()
