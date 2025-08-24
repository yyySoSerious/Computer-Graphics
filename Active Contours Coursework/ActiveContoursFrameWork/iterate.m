function [newX, newY] = iterate(Ainv, x, y, Eext, gamma, kappa)
height = size(Eext, 1);
width = size(Eext, 2);

%% Get fx and fy
Dx = [-1 1]; %1st derivative along x
Dy = [-1; 1]; %1st derivative along y

% convolute
fx = conv2(Eext, Dx, 'same');
fy = conv2(Eext, Dy, 'same');
%%

%retrieve integer parts of x and y
i_vec = floor(x);
j_vec = floor(y);

%retrieve decimal part of x and y
u_vec = x - i_vec;
v_vec = y - j_vec;

%% interpolate fx and fy values
fx_val = zeros(size(x));
fy_val = zeros(size(y));
for index = 1:size(x, 2)
    i = i_vec(1, index);
    j = j_vec(1, index);
    u = u_vec(1, index); 
    v = v_vec(1, index); 
    fx_val(1, index) = (1-u)*v * fx(j + 1, i) + (1-u)*(1-v)*fx(j, i) + u*v * fx(j +1, i +1) + u*(1 -v)*fx(j, i + 1);
    fy_val(1, index) = (1-u)*v * fy(j + 1, i) + (1-u)*(1-v)*fy(j, i) + u*v * fy(j +1, i +1) + u*(1 -v)*fy(j, i + 1);
end
%%

%% Iterate 
newX = Ainv *(gamma*x + kappa * fx_val).';
newY = Ainv * (gamma*y + kappa * fy_val).';
newX = newX.';
newY = newY.';
%%

%% Clamp to image size
newX(newX < 1.0) = 1.01;
newX(newX > width) = width-0.01;
newY(newY < 1.0) = 1.01;
newY(newY > height) = height-0.01;
%%

end

