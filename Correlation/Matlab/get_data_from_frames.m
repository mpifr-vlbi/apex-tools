function dr = get_data_from_frames(r_frames)

nframes = length(r_frames);
nchan   = r_frames{1}.log2_chans^2;
if nchan==0
    nchan=1;
end

% concat data from frames
    dr={};
    for ch=1:nchan
        dr{ch}=[];
        for f=1:nframes
            dr{ch} = [dr{ch} r_frames{f}.ch{ch}.data];
        end

    end    