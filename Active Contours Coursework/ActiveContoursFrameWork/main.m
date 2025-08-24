clear all;

% Parameters (play around with different images and different parameters)
N = 250;
alpha = 0.2;
beta = 0.1;
gamma = 0.35;
kappa = 0.25;
Wline = -0.01;
Wedge = 8.0;
Wterm = 8.6;
sigma = 5.5;

% Load image
I = imread('images/circle.jpg');
% I = imread('images/shape.png');
% I = imread('images/square.jpg');
% I = imread('images/star.png');
% I = imread('images/vase.tif');
% I = imread('images/dental.png');
% I = imread('images/brain.png');

if (ndims(I) == 3)
    I = rgb2gray(I);
end

% Initialize the snake
[x, y] = initializeSnake(I);

% Calculate external energy
I_smooth = double(imgaussfilt(I, sigma));
Eext = getExternalEnergy(I_smooth,Wline,Wedge,Wterm);

% Calculate matrix A^-1 for the iteration
Ainv = getInternalEnergyMatrixBonus(size(x,2), alpha, beta, gamma);

% Iterate and update positions
displaySteps = floor(N/10);
for i=1:N
    % Iterate
    [x,y] = iterate(Ainv, x, y, Eext, gamma, kappa);

    % Plot intermediate result
    imshow(I); 
    hold on;
    plot([x x(1)], [y y(1)], 'r');
        
    % Display step
    if(mod(i,displaySteps)==0)
        fprintf('%d/%d iterations\n',i,N);
    end
    
    pause(0.0001)
end
 
if(displaySteps ~= N)
    fprintf('%d/%d iterations\n',N,N);
end