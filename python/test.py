#!/usr/bin/env python3
from PIL import Image
import random
import os

os.chdir('/home/a/projects/gp_cpp/python')

from genpattern import GPImgAlpha, gp_genpattern



def load_and_modify_image(filepath, copies=1, min_scale=1, max_scale=2, max_rotation=360):
    """
    Load a PNG image, randomly upscale and rotate it, then extract its alpha values.
    Returns a tuple of (GPImgAlpha, PIL.Image) for each copy.
    """
    with Image.open(filepath) as img:
        # Ensure the image has an alpha channel
        if img.mode not in ["RGBA", "LA"]:
            raise ValueError("The image at '%s' doesn't have an alpha channel." % filepath)
        
        result = []

        for _ in range(copies):
            # Randomly upscale and rotate the image
            scale_factor = random.randint(min_scale, max_scale)
            rotation_angle = random.uniform(0, max_rotation)
            
            scaled_img = img.resize((int(img.width * scale_factor), int(img.height * scale_factor)), resample=Image.Resampling.NEAREST)
            rotated_img = scaled_img.rotate(rotation_angle, expand=True, resample=Image.Resampling.BILINEAR)

            # Extract the alpha channel
            if rotated_img.mode == "RGBA":
                alpha_data = rotated_img.split()[3]  # Get the alpha band
            else:  # LA mode
                alpha_data = rotated_img.split()[1]  # Get the alpha band
            
            data = alpha_data.tobytes()
            img_alpha = GPImgAlpha(rotated_img.width, rotated_img.height, data[:])
            result.append((img_alpha, rotated_img))

    return result

def main():
    images = ["test/image1.png", "test/image2.png"]

    WIDTH = 1024
    HEIGHT = 1024
    COPIES = 1024

    # Create collections
    collections = [load_and_modify_image(path, copies=COPIES) for path in images]
    alphas = [[item[0] for item in coll] for coll in collections]
    result = gp_genpattern(alphas, WIDTH, HEIGHT, 64, 5, 5)

    # Create a blank canvas
    canvas = Image.new("RGBA", (WIDTH, HEIGHT), (255, 255, 255, 255))

    # For each set of coordinates, paste the corresponding image onto the canvas
    for coll_idx, coll in enumerate(result):
        for idx, coords in enumerate(coll):
            rotated_img = collections[coll_idx][idx][1]
            for coord in coords:
                canvas.paste(rotated_img, coord, mask=rotated_img)

    # Show the canvas
    canvas.show()
    canvas.save('test.png')

if __name__ == "__main__":
    main()
