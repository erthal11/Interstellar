%%% grpdelay2phase.m

%%% get the phase from the group delay

function ph = grpdelay2phase(grd)

% ph = zeros(1,N);
% ph(1) = -grd(1);
% for i = 2:N
%     ph(i) = -(-ph(i-1)+grd(i));
% end
ph = -cumsum(grd);
ph = 2*pi*ph/length(grd);
