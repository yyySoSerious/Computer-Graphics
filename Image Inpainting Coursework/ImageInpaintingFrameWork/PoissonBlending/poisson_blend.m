function imgout = poisson_blend(im_s, mask_s, im_t)
% -----Input
% im_s     source image (object)
% mask_s   mask for source image (1 meaning inside the selected region)
% im_t     target image (background)
% -----Output
% imgout   the blended image

imgout = im_t;
[imh, imw, nb] = size(im_s);
selPixelLoc = find(mask_s); % get indices of selected region
numSelectedPixels = size(selPixelLoc, 1);
b = zeros(numSelectedPixels, 1);
rows = zeros(1, numSelectedPixels * 5);
cols = zeros(1, numSelectedPixels * 5);
values = zeros(1, numSelectedPixels * 5);

for channel =  1 : nb
    im_s_channel = im_s(:, :, channel);
    im_t_channel = im_t(:, :, channel);
    row = 1;

    %process each channel of the image
    for pixel = 1 : numSelectedPixels
        rows(row : row + 4) = pixel;
        total_constant = 0;

        if(mask_s(selPixelLoc(pixel) - imh) == 1) %left neighbour pixel
            cols(row) = find(selPixelLoc == (selPixelLoc(pixel) - imh));
            values(row) = -1;
        else
            cols(row) = pixel; %now, j in A(i, j) will be non-zero
            values(row) = 0;
            total_constant = total_constant + im_t_channel(selPixelLoc(pixel) - imh);
        end
        
        if(mask_s(selPixelLoc(pixel) - 1) == 1) %top neighbour pixel
            cols(row + 1) = find(selPixelLoc == selPixelLoc(pixel) - 1);
            values(row + 1) = -1;
        else
            cols(row + 1) = pixel;
            values(row + 1) = 0;
            total_constant = total_constant + im_t_channel(selPixelLoc(pixel) - 1);
        end

        if(mask_s(selPixelLoc(pixel) + 1) == 1) %bottom neighbour pixel
            cols(row + 2) = find(selPixelLoc == selPixelLoc(pixel) + 1);
            values(row + 2) = -1;
        else
            cols(row + 2) = pixel;
            values(row + 2) = 0;
            total_constant = total_constant + im_t_channel(selPixelLoc(pixel) + 1);
        end

        if(mask_s(selPixelLoc(pixel) + imh) == 1) % right neighbour pixel
            cols(row + 3) = find(selPixelLoc == selPixelLoc(pixel) + imh);
            values(row + 3) = -1;
        else
            cols(row + 3) = pixel;
            values(row + 3) = 0;
            total_constant = total_constant + im_t_channel(selPixelLoc(pixel) + imh);
        end
        cols(row + 4) = pixel;
        values(row + 4) = 4;
        b(pixel, 1) = -im_s_channel(selPixelLoc(pixel)-imh) -im_s_channel(selPixelLoc(pixel)-1) + 4*im_s_channel(selPixelLoc(pixel)) - im_s_channel(selPixelLoc(pixel) + 1) - im_s_channel(selPixelLoc(pixel)+imh) + total_constant;
        row = row + 5;
    end
    A = sparse(rows, cols, values);
    solution = lscov(A, b); % A\b;
    error = sum(abs(A*solution-b));
    disp(error)
    imgout_channel = im_t_channel;
    imgout_channel(selPixelLoc(1:numSelectedPixels)) = solution(1:numSelectedPixels);
    imgout(:, :, channel) = imgout_channel;
end
