clc;
clear;
close all;

imgin = im2double(imread('./large.jpg'));

[imh, imw, nb] = size(imgin);
assert(nb==1);
% the image is grayscale

V = zeros(imh, imw);
V(1:imh*imw) = 1:imh*imw;
% V(y,x) = (y-1)*imw + x
% use V(y,x) to represent the variable index of pixel (x,y)

totalPixels = imh * imw;
midPixelsLength = imh - 2; %number of pixels in a column of an image that is not on the edge of the image
numRows = (totalPixels - 4) * 3 + 4 + 4;
rows = zeros(1, numRows);
cols = zeros(1, numRows);
values = zeros(1, numRows);
b = zeros(totalPixels + 4, 1);

row  = 1; %current row in rows vector, skipping top-left corner pixel

%% top-left corner pixel
rows(row) = 1;
cols(row) = 1;
values(row) = 0;
b(V(1,1)) = 0;
%%

%move row index
row = row + 1;
i = row; %current row in A matrix or input image

%% left Edge
for i = i : i + midPixelsLength - 1
    rows(row : row+2) = i;
    cols(row : row+2) = V(i-1 : i+1);
    values(row) = -1;
    values(row+1) = 2;
    values(row+2) = -1;
    row = row + 3;
    b(i, 1) = -imgin(i-1) + 2*imgin(i) - imgin(i + 1);
end

%% bottom-left corner pixel
rows(row) = imh;
cols(row) = 1;
values(row) = 0;
b(V(imh,1)) = 0;
%%

%start next column of input image by moving row index
row = row + 1;
i = i + 2;

%Loop through the cols of input image minus the last column
for imgcols = 2: imw-1
    %% top edge value
    rows(row : row+2) = i;
    cols(row) = V(i - imh);
    cols(row + 1) = V(i);
    cols(row + 2) = V(i + imh);
    values(row) = -1;
    values(row + 1) = 2;
    values(row + 2) = -1;
    b(i, 1) = -imgin(i-imh) + 2*imgin(i) - imgin(i + imh);
    %%
    row = row + 3;
    i = i + 1;

    %% middle pixels
    for i = i : i + midPixelsLength - 1
        rows(row : row + 4) = i;
        cols(row) = V(i - imh);
        cols(row + 1) = V(i-1);
        cols(row + 2) = V(i);
        cols(row + 3) = V(i + 1);
        cols(row + 4) = V(i + imh);
        values(row : row + 1) = -1;
        values(row + 2) = 4;
        values(row+3 : row+4) = -1;
        b(i, 1) = -imgin(i-imh) -imgin(i-1) + 4*imgin(i) - imgin(i + 1) - imgin(i+imh);
        row = row + 5;
    end
    %%

    i = i + 1;

    %% bottom edge val
    rows(row : row + 2) = i;
    cols(row) = V(i - imh);
    cols(row + 1) = V(i);
    cols(row + 2) = V(i + imh);
    values(row) = -1;
    values(row + 1) = 2;
    values(row + 2) = -1;
    b(i, 1) = -imgin(i-imh) + 2*imgin(i) - imgin(i + imh);
    %%
    row = row + 3;
    i = i + 1;
end

%% top-right corner pixel
rows(row) = 1;
cols(row) = imw;
values(row) = 0;
b(V(1,imw)) = 0;
%%

%move row index
row = row + 1;
i = i + 1;

%% right edge
for i = i : i + midPixelsLength -1
    rows(row : row + 2) = i;
    cols(row : row+2) = V(i-1 : i+1);
    values(row) = -1;
    values(row + 1) = 2;
    values(row +2) = -1;
    row = row + 3;
    b(i, 1) = -imgin(i-1) + 2*imgin(i) - imgin(i + 1);
end
%%

%% bottom-right corner pixel
rows(row) = imh;
cols(row) = imw;
values(row) = 0;
b(V(imh,imw)) = 0;
%%

%move row index
row = row + 1;
i = i + 2;

%% add extra constaints
rows(row : row + 3) = totalPixels+1 : totalPixels+4;
cols(row) = V(1, 1);
cols(row + 1) = V(1, imw);
cols(row + 2) =  V(imh, 1);
cols(row + 3) =  V(imh, imw);
values(row) = 1;
values(row + 1) = 1;
values(row + 2) = 1;
values(row + 3) = 1;
b(totalPixels + 1) =  imgin(1, 1);
b(totalPixels + 2) = imgin(1, imw);
b(totalPixels + 3) = imgin(imh, 1);
b(totalPixels + 4) =  imgin(imh, imw);
%%

% initialize A
A = sparse(rows, cols, values);

%TODO: solve the equation
%use "lscov" or "\", please google the matlab documents
solution = A\b;
error = sum(abs(A*solution-b));
disp(error);
imgout = reshape(solution,[imh,imw]);

imwrite(imgout,'output.png');
figure(), hold off, imshow(imgout);

