function [frames,d] = read_n_r2_frames(file,n2frames,skip_frames)

    %pkt_len_us = 8;
    pkt_B = 8224;

    bytes_skip = skip_frames*pkt_B;

    fid = fopen(file,'r');
    fseek(fid,bytes_skip,'bof');

    for f = 1:n2frames
        [frame,fid] = read_r2_frame(fid,pkt_B);
        frames{f} = frame;
    end
    fclose(fid);

end

function [frame,fid] = read_r2_frame(fid,len_Bytes)

    % R2DBE 125000 frames per second for 1x2048 MHz IF
    fs = 2*2048e6; nbit = 2; pkt_payload = 8192;
    fps = fs*nbit / (8*pkt_payload);

    frame = struct();

    [frame,fid,count]=read_vdif_hdr_r2(fid,frame);

    hdr_len = count; % 8 32 bit words
    data_len = (len_Bytes/4) - hdr_len;

    frame.datetime = get_date_from_vdif(frame.secs_since_epoch,frame.ref_epoch,frame.data_frame,fps);
    frame.Fs = 4096e6;
    frame.len_us = 1/fps*1e6;
    frame.frames_per_sec = fps;
    frame.is_flipped = 0;
    frame.samp_per_frame = 32768;


    [frame,fid] = read_vdif_data_r2(fid,frame,data_len);
end

function [frame,fid,count] = read_vdif_hdr_r2(fid,frame)

    [words,count1]=fread(fid,4,'uint32', 0, 'ieee-le');

    % word 0
    frame.invalid_data     = boolean(bitand(bitshift(words(1),-31),hex2dec('1')));
    frame.legacy_mode      = boolean(bitand(bitshift(words(1),-30),hex2dec('1')));
    frame.secs_since_epoch = bitand(words(1),hex2dec('3fffffff'));

    % word 1
    frame.ref_epoch  = bitand(bitshift(words(2),-24),hex2dec('3f'));
    frame.data_frame = bitand(words(2),hex2dec('ffffff'));

    % word 2
    frame.vdif_vers    = bitand(bitshift(words(3),-29),hex2dec('7'));
    frame.log2_chans   = bitand(bitshift(words(3),-24),hex2dec('1f'));
    frame.frame_length = bitand(words(3),hex2dec('ffffff'));

    % word 3
    frame.complex         = boolean(bitand(bitshift(words(4),-31),hex2dec('1')));
    frame.bits_per_sample = 1 + bitand(bitshift(words(4),-26),hex2dec('1f'));
    frame.thread_id       = bitand(bitshift(words(4),-16),hex2dec('3ff'));
    frame.station_id      = bitand(words(4),hex2dec('ffff'));

    % parse extended user data

    count2 = 0;
    % words 4-7
    if ~(frame.legacy_mode) % if not leg mode, more data to read
        [words(5:8),count2] = fread(fid,4,'uint32', 0, 'ieee-le');
        frame.eud_vers = bitand(bitshift(words(5),-24),hex2dec('ff'));
        frame.PSN_high = words(7);
        frame.PSN_low  = words(8);
    end

    count = count1+count2;
end

function [frame,fid] = read_vdif_data_r2(fid,frame,data_len)

    tmp = fread(fid,data_len,'uint32', 0, 'ieee-le');

    dat = [];
    for ch= 1:1:16
       dat  = [dat; bitand(bitshift(tmp,-2*(ch-1)), hex2dec('3')).'];

    end

    vals = [-3 1 -1 3] ;
	% JanW: seems wrong encoding for VDIF?  should be --,-,+,++ e.g. [-7, -2, +2, +7] ?
    vals = [-3.3359,-1.0,+1.0,+3.3359];

    dat=vals(dat+1);
    dat = dat(:).';

    for ch = 1:1
       chlo = 0; %range in MHz
       chhi = 2048; % MHz
       % do like

       frame.ch{ch}.flo = chlo;
       frame.ch{ch}.fhi = chhi;
       frame.ch{ch}.data = dat;
    end
end
