function timestamp = get_date_from_vdif(sec_since_ref_ep, ref_ep, framenum, fps)

% get year from ref_ep
if mod(ref_ep,2)==1
    yr = floor(ref_ep/2)+2000;
    ref_datenum = datenum(yr,7,1,0,0,0);
else
    yr = floor(ref_ep/2)+2000;
    ref_datenum = datenum(yr,1,1,0,0,0);
end

secs = datenum(0,0,0,0,0,sec_since_ref_ep);

date_info = datevec(ref_datenum+secs);

timestamp.year = date_info(1);
timestamp.month = date_info(2);
timestamp.day   = date_info(3);
timestamp.hour  = date_info(4);
timestamp.min   = date_info(5);
timestamp.sec   = date_info(6);

% calculate milliseconds

ms_per_frame = 1000/fps;


timestamp.ms = framenum*ms_per_frame;