%
% Octave script to load data from pollCounters.out (clock offsets, e.g. 1PPS offsets),
% show a plot of the offset over time, and fit a line.
%

num_header_rows = 5;
dd = dlmread('pollCounters.out', '', num_header_rows,0);
t = dd(:,1);
d = dd(:,2);

sigma_t = 1/10e6;
sigma_d = 1/10e6;
err_corr_r = 1.0; % assume fully correlated error in measuring timestamp and clock offset
[a,b,sigma_a,sigma_b,b_save] = york_fit(t',d',sigma_t,sigma_d,err_corr_r);
fit_d = a + b*t;
fprintf(1, 'Rate by York regression : %.12e\n', b);

Th = 60*60;

figure(1); hold on;
  plot(t/Th,d, 'kx');
  plot(t/Th,fit_d, 'r-');
  xlabel('Time (hours)')
  ylabel('Time (s)')
  legend('Data', ['York regression, best-fit rate (' num2str(b,'%.12e') ' s/s)']);
