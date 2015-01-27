function pollCounters_glue_logs(varargin)
%
% Octave script to load data from a list of log files
% produced by 'pollCounters' and combines the data into
% a single output log file.
%

if nargin<=1,
  fprintf(1, "Usage: pollCounters_glue_logs('file1.log', 'file2.log', ...)\n");
  return;
end

Nfiles = nargin;

t_start = {};
fdo = fopen('pollCounters.log.glued', 'w');

for ii=1:Nfiles,
  filename = varargin{ii};
  fprintf('Loading %s ...\n', filename);

  num_header_rows = 5;
  dd = dlmread(filename, '', num_header_rows,0);
  t = dd(:,1);
  d = dd(:,2);

  % Log files after 5 Jan 2015 have the starting time in
  % the header, in a 2015y001d12h30m00s kind of format.
  % Shift the times t (seconds timestamps) by adding
  % this date as a Unix timestamp (seconds since 1/1/1970).
  fd = fopen(filename,'r');
  tmp = fgetl(fd);
  txt = fgetl(fd);  % "Started at UT 2015y008d03h51m25s"
  fclose(fd);

  tstr = txt(15:end);
  [tm,nchar] = strptime(tstr, '%Yy%jd%Hh%Mm%Ss');  
  % note: tm.zone will be timezone of Mark6, but will be treated as UTC

  if isempty(t_start),
     t_start = tm;
     fprintf(fdo, '--------------------------------\n');
     fprintf(fdo, 'Started at UT %s\n', tstr);
     fprintf(fdo, 'Elapsed      Time Interval\n');
     fprintf(fdo, 'Time / s          / s\n');
     fprintf(fdo, '--------------------------------\n');
  end

  t_start_diff_sec = mktime(tm) - mktime(t_start);
  for mm=1:numel(t),
  % "3112           8.465615467e-01"
    fprintf(fdo, '%-11d %.9e\n', t(mm)+t_start_diff_sec, d(mm));
  end

end
fclose(fdo);

