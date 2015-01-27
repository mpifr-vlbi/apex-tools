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

if 0,
  % Log files after 5 Jan 2015 have the starting time in
  % the header, in a 2015y001d12h30m00s kind of format.
  % Shift the times t (seconds timestamps) by adding
  % this date as a Unix timestamp (seconds since 1/1/1970).
  fd = fopen(filename,'r');
  tmp = fgetl(fd);
  txt = fgetl(fd);  % "Started at UT 2015y008d03h51m25s"
  % TODO
end

sigma_t = 1/10e6;
sigma_d = 1/10e6;
err_corr_r = 1.0; % assume fully correlated error in measuring timestamp and clock offset
[a,b,sigma_a,sigma_b,b_save] = york_fit(t',d',sigma_t,sigma_d,err_corr_r);
fit_d = a + b*t;
fit_e = d - fit_d;
fprintf(1, 'Rate by York regression : %.12e\n', b);

figure(1); clf; hold on;
  thours = t / 3600;
  tYoffset = mean(d);
  subplot(2,1,1),
    plot(thours,d-tYoffset, 'kx'), hold on;
    plot(thours,fit_d-tYoffset, 'r-');
    xlabel('Time (hours)')
    ylabel(['Time (s) as an offset from ' num2str(tYoffset,'%.12f')])
    xlim([min(thours) max(thours)]);
    legend('Data', ['York regression, best-fit rate (' num2str(b,'%.12e') ' s/s)']);
    fntidy = strrep(filename, '_', '\_');
    title(['Data and fit for ' fntidy]);
  subplot(2,1,2),
    plot(thours, fit_e, 'kx');
    xlabel('Time (hours)')
    ylabel('Residual')
    xlim([min(thours) max(thours)]);
    

  % Fill PDF page
  border = 0.25; 
  papersize = get(gcf(), 'papersize');
  set (gcf(), 'paperposition', [border, border, (papersize - 2*border)]);

  % To change orientation 
  orientation = get (gcf(), 'paperorientation'); 
  papersize = get (gcf(), 'papersize'); 
  paperposition = get (gcf(), 'paperposition'); 
  set (gcf(), 'paperposition', paperposition([2, 1, 4, 3])); 
  set (gcf(), 'papersize', papersize ([2, 1])); 
  set (gcf(), 'paperorientation', 'landscape');

  print('pollCounters_fit_rate.pdf', '-dpdf');
  fprintf(1, 'Wrote plot to file  pollCounters_fit_rate.pdf\n');

