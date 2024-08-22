import numpy as np
from PIL import Image

def image_to_argb(image_path):
    img = Image.open(image_path).convert("RGBA")
    img_array = np.array(img, dtype=np.uint8)
    
    argb_array = []
    for y in range(img_array.shape[0]):
        for x in range(img_array.shape[1]):
            r, g, b, a = img_array[y, x]
            a, r, g, b = int(a), int(r), int(g), int(b)

            argb = (a << 24) | (b << 16) | (g << 8) | r
            argb_array.append(argb)
    
    return argb_array

def generate_cpp_array(var_name, argb_array):
    cpp_code = f"std::array<uint32_t, {len(argb_array)}> {var_name} = {{\n"
    for i, argb in enumerate(argb_array):
        cpp_code += f"0x{argb:08X}"
        if i != len(argb_array) - 1:
            cpp_code += ", "
        if (i + 1) % 8 == 0:
            cpp_code += "\n"
    cpp_code += "\n};"
    return cpp_code

def convert_image_to_cpp(image_path, var_name):
    argb_array = image_to_argb(image_path)
    cpp_code = generate_cpp_array(var_name, argb_array)
    return cpp_code

if __name__ == '__main__':
    blue = convert_image_to_cpp( 'orb_big_20.png', 'orb_data')
    print(blue)
