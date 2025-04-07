#!/usr/bin/env python3
from PIL import Image
import random
import os
from typing import List, Tuple

from genpattern_lib import GPImgAlpha, gp_genpattern, GPExponentialSchedule, GPLinearSchedule, SCRIPT_PATH

os.chdir(SCRIPT_PATH)

def load_and_modify_image(
    filepath: str,
    copies: int = 1,
    min_scale: float = 1.0,
    max_scale: float = 2.0,
    max_rotation: int = 360
) -> List[Tuple[GPImgAlpha, Image.Image]]:
    """
    Load a PNG image, randomly upscale and rotate it, then extract its alpha channel.
    Returns a list of tuples (GPImgAlpha, PIL.Image) for each copy.
    """
    with Image.open(filepath) as img:
        if img.mode not in ["RGBA", "LA"]:
            raise ValueError(f"The image at '{filepath}' doesn't have an alpha channel.")
        
        results: List[Tuple[GPImgAlpha, Image.Image]] = []
        for _ in range(copies):
            scale_factor: float = random.uniform(min_scale, max_scale)
            rotation_angle: int = random.randint(0, max_rotation)
            
            # Resize and rotate.
            scaled_img = img.resize(
                (int(img.width * scale_factor), int(img.height * scale_factor)),
                resample=Image.Resampling.NEAREST
            )
            rotated_img = scaled_img.rotate(
                rotation_angle,
                expand=True,
                resample=Image.Resampling.NEAREST
            )
            
            # Extract the alpha channel.
            if rotated_img.mode == "RGBA":
                alpha_data = rotated_img.split()[3]
            else:  # LA mode
                alpha_data = rotated_img.split()[1]
            
            data: bytes = alpha_data.tobytes()
            img_alpha = GPImgAlpha(rotated_img.width, rotated_img.height, data)
            results.append((img_alpha, rotated_img))
    
    return results

def main() -> None:
    SEED: int = 42
    random.seed(SEED)

    # List of image file paths.
    # Uncomment and adjust paths as needed.
    # images: List[str] = ["test/a1.png", "test/a2.png", "test/a3.png", "test/a4.png", "test/a5.png"]
    images: List[str] = ["test/b1.png", "test/b2.png", "test/b3.png", "test/b4.png"]

    WIDTH: int = 512
    HEIGHT: int = 512
    COPIES: int = 100

    # Load and modify images.
    collections_raw = [load_and_modify_image(path, copies=COPIES) for path in images]
    # Extract only GPImgAlpha instances for binding.
    alphas: List[List[GPImgAlpha]] = [[item[0] for item in coll] for coll in collections_raw]

    # Call the binding's gp_genpattern function.
    result = gp_genpattern(
        alphas,
        WIDTH,
        HEIGHT,
        threshold=64,
        offset_radius=5,
        collection_offset_radius=20,
        schedule=GPExponentialSchedule(0.8),
        seed=SEED
    )

    # Create a blank canvas.
    canvas = Image.new("RGBA", (WIDTH, HEIGHT), (255, 255, 255, 255))

    # Paste images onto the canvas at the coordinates provided by gp_genpattern.
    for coll_idx, coll in enumerate(result):
        for idx, coords in enumerate(coll):
            rotated_img = collections_raw[coll_idx][idx][1]
            for coord in coords:
                canvas.paste(rotated_img, coord, mask=rotated_img)

    # Display and save the final image.
    canvas.show()
    canvas.save('test.png')

if __name__ == "__main__":
    main()
