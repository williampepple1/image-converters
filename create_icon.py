"""
Create an icon for the Image Converter application
"""
from PIL import Image, ImageDraw, ImageFont
import os

def create_icon():
    # Create icons at multiple sizes
    sizes = [256, 128, 64, 48, 32, 16]
    images = []
    
    for size in sizes:
        # Create a new image with transparency
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # Background - rounded rectangle effect with gradient-like colors
        padding = size // 16
        
        # Draw main background circle/rounded shape
        bg_color = (137, 180, 250)  # #89b4fa - Catppuccin blue
        draw.ellipse([padding, padding, size - padding, size - padding], fill=bg_color)
        
        # Draw inner circle
        inner_padding = size // 6
        inner_color = (30, 30, 46)  # #1e1e2e - Catppuccin base
        draw.ellipse([inner_padding, inner_padding, size - inner_padding, size - inner_padding], fill=inner_color)
        
        # Draw arrows to represent conversion
        arrow_color = (166, 227, 161)  # #a6e3a1 - Catppuccin green
        center = size // 2
        arrow_size = size // 4
        
        # Right arrow (convert to)
        arrow_points = [
            (center - arrow_size // 2, center - arrow_size // 3),
            (center + arrow_size // 2, center),
            (center - arrow_size // 2, center + arrow_size // 3),
        ]
        draw.polygon(arrow_points, fill=arrow_color)
        
        # Left side - image icon representation
        img_icon_color = (243, 139, 168)  # #f38ba8 - Catppuccin pink
        rect_size = size // 5
        rect_x = center - arrow_size - rect_size // 2
        rect_y = center - rect_size // 2
        draw.rectangle([rect_x, rect_y, rect_x + rect_size, rect_y + rect_size], fill=img_icon_color)
        
        # Small triangle in the image icon (mountain)
        if size >= 32:
            mt_points = [
                (rect_x + rect_size // 4, rect_y + rect_size * 3 // 4),
                (rect_x + rect_size // 2, rect_y + rect_size // 3),
                (rect_x + rect_size * 3 // 4, rect_y + rect_size * 3 // 4),
            ]
            draw.polygon(mt_points, fill=inner_color)
        
        images.append(img)
    
    # Save as ICO with multiple sizes
    images[0].save(
        'app_icon.ico',
        format='ICO',
        sizes=[(s, s) for s in sizes],
        append_images=images[1:]
    )
    
    # Also save a PNG for other uses
    images[0].save('app_icon.png', format='PNG')
    
    print(f"Created app_icon.ico and app_icon.png")

if __name__ == '__main__':
    create_icon()
