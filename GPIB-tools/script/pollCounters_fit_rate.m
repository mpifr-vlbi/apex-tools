function pollCounters_fit_rate(filename)
%
% Octave script to load data from pollCounters.out (clock offsets, e.g. 1PPS offsets),
% show a plot of the offset over time, and fit a line.
% File name is optional, defaults to 'pollCounters.out'. 
%

if nargin<1,
  filename = 'pollCounters.out';
end

num_header_rows = 5;
dd = dlmread(filename, '', num_header_rows,0);
t = dd(:,1);
d = dd(:,2);

sigma_t = 1/10e6;
sigma_d = 1/10e6;
err_corr_r = 1.0; % assume fully correlated error in measuring timestamp and clock offset
[a,b,sigma_a,sigma_b,b_save] = york_fit(t',d',sigma_t,sigma_d,err_corr_r);
fit_d = a + b*t;
fprintf(1, 'Rate by York regression : %.12e\n', b);

figure(1); clf; hold on;
  thours = t / 3600;
  plot(thours,d, 'kx');
  plot(thours,fit_d, 'r-');
  xlabel('Time (hours)')
  ylabel('Time (s)')
  xlim([min(thours) max(thours)]);
  legend('Data', ['York regression, best-fit rate (' num2str(b,'%.12e') ' s/s)']);
  fntidy = strrep(filename, '_', '\_');
  title(['Data and fit for ' fntidy]);

