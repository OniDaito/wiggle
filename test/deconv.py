# https://stackoverflow.com/questions/20266825/deconvolution-with-opencv
import numpy as np
import scipy.ndimage
import matplotlib.pyplot as plt
import imageio

width = 640
height = 300
depth = 50
imgs = np.zeros((height, width, depth))

# Read in the stacked tiffs
for k in range(depth):
    imgs[:, :, k] = imageio.imread("../images/stack/worm3d%04d.png" % (k))

# prepare output array, top and bottom image in stack don't get filtered
out_imgs = np.zeros_like(imgs)
out_imgs[:, :, 0] = imgs[:, :, 0]
out_imgs[:, :, -1] = imgs[:, :, -1]

# apply nearest neighbor deconvolution
alpha = 2.0  # adjustabe parameter, strength of filter
sigma_estimate = 8  # estimate, just happens to be same as the actual

for k in range(1, depth - 1):
    # subtract blurred neighboring planes in the stack from current plane
    # doesn't have to be gaussian, any other kind of blur may be used: this should approximate PSF
    out_imgs[:, :, k] = (1 + alpha) * imgs[:, :, k]  \
        - (alpha / 2) * scipy.ndimage.filters.gaussian_filter(imgs[:, :, k - 1], sigma_estimate) \
        - (alpha / 2) * scipy.ndimage.filters.gaussian_filter(imgs[:, :, k + 1], sigma_estimate)

input_image = np.sum(imgs, axis=2)
output_image = np.sum(out_imgs, axis=2)

fig, ((ax1, ax2)) = plt.subplots(nrows=2, ncols=1)
ax1.imshow(input_image)
ax2.imshow(output_image)
plt.show()