function [Ainv] = getInternalEnergyMatrixBonus(nPoints, alpha, beta, gamma)

A = zeros(nPoints);
for i = 1 : nPoints
    A(i, i) = 2 * alpha + 6 * beta;
    A(i, mod(i+ 1-1, nPoints)+1) = -alpha - 4*beta;
    A(i, mod(i + 2-1, nPoints)+1) = beta;
    A(i, mod(i-1-1, nPoints) + 1) =  -alpha - 4*beta;
    A(i, mod(i-2-1, nPoints) + 1) = beta;
end

Ainv = inv(A + gamma*eye(nPoints));
end

