% function [init_dly_us] = find_initial_offset(d1,d2)
%
% Returns the time offset in microseconds between two timestamps.
% Timestamps are structs in the format returned by get_date_from_vdif().
%
function [init_dly_us] = find_initial_offset(d1,d2)

d1_vec = [d1.year, d1.month, d1.day, d1.hour, d1.min, d1.sec];
d2_vec = [d2.year, d2.month, d2.day, d2.hour, d2.min, d2.sec];

sdiff  = etime(d2_vec,d1_vec); % returns seconds between dates
msdiff = d2.ms-d1.ms;  % positive is d2 is behind d1, negative if d2 is ahead of d1
init_dly_us = sdiff*1e6 + msdiff*1e3;
