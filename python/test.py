#!/usr/bin/env python3
from PIL import Image
import copy

from genpattern import GPImgAlpha, gp_genpattern


def load_image_as_gp_img_alpha(filepath, copies=1):
    """
    Load a PNG image and extract its alpha values, then create a GPImgAlpha instance 
    with the option to load multiple copies.
    """
    with Image.open(filepath) as img:

        # Ensure the image has an alpha channel
        if img.mode not in ["RGBA", "LA"]:
            raise ValueError("The image at '%s' doesn't have an alpha channel." % filepath)

        width, height = img.size

        # Extract the alpha channel
        if img.mode == "RGBA":
            alpha_data = img.split()[3]  # Get the alpha band
        else:  # LA mode
            alpha_data = img.split()[1]  # Get the alpha band

    data = alpha_data.tobytes()
    result = []
    for _ in range(copies):
        img_alpha = GPImgAlpha(width, height, data[:])
        result.append(img_alpha)

    return result

def main():
    # Load 5 copies of image1.png and 3 copies of image2.png
    images = ["test/image1.png", "test/image2.png"]

    # Create collections
    collections = [load_image_as_gp_img_alpha(path, copies=40) for path in images]
    result = gp_genpattern(collections, 129, 128, 64, 10, 16)

    # Create a blank canvas
    canvas = Image.new("RGBA", (128, 128), (255, 255, 255, 255))

    # For each set of coordinates, paste the corresponding image onto the canvas
    for coll_idx, coll in enumerate(result):
        for coords in coll:
            print(coords)
            for coord in coords:
                with Image.open(images[coll_idx]) as pil_img:
                    canvas.paste(pil_img, coord, mask=pil_img)


    # Show the canvas
    canvas.show()

    

if __name__ == "__main__":
    main()
