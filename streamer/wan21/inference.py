from __future__ import annotations

import numpy as np


def inference_process(
    input_image: np.ndarray,
    count: int,
    output_image: np.ndarray,
    scale: int | None,
    y_indices: np.ndarray | None,
    x_indices: np.ndarray | None,
) -> None:
    """Process one RGBA frame into a small AI preview image.

    input_image is a height * width * 4 uint8 RGBA array.
    output_image is an output_height * output_width * 4 uint8 RGBA array.
    count is SharedBuffer.count from /dev/shm/reciever.dat.
    """
    del count
    if scale is not None:
        output_image[:, :, :] = input_image[::scale, ::scale, :]
        return

    if y_indices is None or x_indices is None:
        raise ValueError("scale or downsample indices are required")
    output_image[:, :, :] = input_image[y_indices[:, None], x_indices[None, :], :]
