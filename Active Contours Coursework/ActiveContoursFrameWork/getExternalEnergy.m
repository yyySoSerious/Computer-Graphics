function [Eext] = getExternalEnergy(I,Wline,Wedge,Wterm)

% Eline
Eline = I;

%% Eedge
[Gmag,~] = imgradient(I);
Eedge = -Gmag.^0.5;
%%

%% Eterm 
% initialize derivative masks
Dx = [-1 1]; %1st derivative along x
Dxx = [1 -2 1]; %2nd derivative along x
Dy = [-1; 1]; %1st derivative along y
Dyy = [1; -2; 1]; %2nd derivative along y
Dxy = [1 0 -1; 0 0 0; -1 0 1]; %1st derivative along x, then along y


% convolute
Cx = conv2(I, Dx, 'same');
Cy = conv2(I, Dy, 'same');
Cxx = conv2(I, Dxx, 'same');
Cyy =  conv2(I, Dyy, 'same');
Cxy =  conv2(I, Dxy, 'same');

Cx2 = Cx.^2; 
Cy2 = Cy.^2;

Eterm = (Cyy.*Cx2 - 2*Cxy.*Cx.*Cy + Cxx.*Cy2)./((1 + Cx2 + Cy2).^(1.5));
%%

% Eext
Eext = Wline*Eline + Wedge*Eedge + Wterm*Eterm;

end

