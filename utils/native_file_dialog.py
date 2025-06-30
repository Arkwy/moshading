import tkinter as tk
from tkinter import filedialog

root = tk.Tk()
root.withdraw()

file_path = filedialog.askopenfilename(
    title="Select image",
    filetypes=[
        ("Image files", "*.jpg *.jpeg *.png *.bmp *.tga *.gif *.psd *.hdr *.pic *.pnm"),
        ("JPEG", "*.jpg *.jpeg"),
        ("PNG", "*.png"),
        ("Bitmap", "*.bmp"),
        ("Targa", "*.tga"),
        ("GIF", "*.gif"),
        ("Photoshop", "*.psd"),
        ("HDR", "*.hdr"),
        ("Softimage PIC", "*.pic"),
        ("PNM/PGM/PPM", "*.pnm"),
    ]
)

print(file_path)
