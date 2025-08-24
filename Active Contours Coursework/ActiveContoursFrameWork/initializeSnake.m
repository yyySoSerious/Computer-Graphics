function [x, y] = initializeSnake(I)
height = size(I, 1);
width = size(I, 2);

% Show figure
imshow(I);

% Get initial points
[x, y] = getpts();

numPts = size(x, 1);

%% Interpolate values between points
angles = linspace(0,2*pi,numPts + 1);
p = [x.' x(1); 
      y.' y(1)];
pp = spline(angles, p);
yy = ppval(pp, linspace(0,2*pi,150));
%%

%% Clamp points to be inside of image
yy(1, yy(1, :) > width) = width-0.01;
yy(2, yy(2, :) > height) = height-0.01;
yy(yy < 1) = 1.01;
x = yy(1, :);
y = yy(2, :);
%%

end

